#include <unistd.h>
#include <sys/stat.h>
#include <chrono>
#include <mpi.h>
#include <random>
// #include <labios.h>
#include "sonar.h"


/*
 *	main - driver for Sonar benchmark
 */
int main(int argc, char** argv)
{
	// benchmark parameters (w/ defaults)
	int rv, opt, t;
	int num_iterations    = DEFAULT_PHASES;            // 5 iterations
	int num_requests      = DEFAULT_REQUESTS;          // 5 dumps
	int num_accesses      = DEFAULT_ACCESSES;          // 1 access (per request)
	int num_reads         = DEFAULT_READS;             // 3 reads
	int num_writes        = DEFAULT_WRITES;            // 1 write
	int access_pattern    = SEQUENTIAL;                // sequential access
	int stride_length     = DEFAULT_STRIDE;            // 10 byte stride
	int intensity         = BSLEEP;                    // busy sleep
	int sleep_time        = DEFAULT_SLEEP;             // 10 seconds
	int io_min            = MIN_IOSIZE * KB;           // 4 KB
	int io_max            = MIN_IOSIZE * KB;           // 4 KB
	char *output_file     = (char *)"./sonar-log.csv"; // default output file

	// check args
	if (argc == 1) {
		showUsage(argv);
		return 0;
	}

	// initialize MPI
	int rank, nprocs;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);   // obtain rank
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs); // obtain number of processes

	// parse given options
	while ((opt = getopt(argc, argv, "h::r:w:a:i:R:n:l:o:s:c:t:")) != EOF) {
		switch (opt) {
			case 'h':
				showUsage(argv);
				return 0;
			case 'r':
				num_reads = std::atoi(optarg);
				break;
			case 'w':
				num_writes = std::atoi(optarg);
				break;
			case 'a':
				t = std::atoi(optarg);
				access_pattern = (t >= MIN_ACCESS && t <= MAX_ACCESS) ? t : access_pattern;
				break;
			case 'i':
				num_iterations = std::atoi(optarg);
				break;
			case 'R':
				num_requests = std::atoi(optarg);
				break;
			case 'n':
				num_accesses = std::atoi(optarg);
				break;
			case 'l':
				stride_length = std::atoi(optarg);
				break;
			case 'o':
				output_file = optarg;
				break;
			case 's':
				t = parseRequestSize(optarg);
				io_min = (t >= MIN_IOSIZE && t <= MAX_IOSIZE) ? t : io_min;
				break;
			case 'S':
				t = parseRequestSize(optarg);
				io_max = (t >= MIN_IOSIZE && t <= MAX_IOSIZE) ? t : io_max;
				break;
			case 'c':
				t = std::atoi(optarg);
				intensity = (t <= MAX_INTENSITY && t >= MIN_INTENSITY) ? t : intensity;
				break;
			case 't':
				sleep_time = std::atoi(optarg);
				break;
			default:
				showUsage(argv);
				break;
		}
	}

	// check that min is <= max
	if (io_max < io_min) {
		std::cerr << "Invalid I/O range: " << io_min << "B - " << io_max << "B\n";
		showUsage(argv);
		return -1;
	}

	// store parameters
	int params[] =
		{
			num_reads,
			num_writes,
			access_pattern,
			num_accesses,
			io_min,
			io_max,
			stride_length
		};

	// store mpi variables
	int mpi[] = {rank, nprocs};

	long *data;
	if (rank == 0) {
		data = (long *) malloc(sizeof(long) * nprocs);
		memset(data, 0, sizeof(long) * nprocs);
	}

	// perform benchmark
	for (int iter = 0; iter < num_iterations; iter++) {
		for (int req = 0; req < num_requests; req++) {
			mainIO(params, mpi, data);

			if (intensity)
				compute(intensity, sleep_time);
		}

		if (intensity)
			compute(intensity, sleep_time);
	}

	if (rank == 0) {
		rv = logData(data, params, nprocs, output_file, 1);
		free(data);
	}

	// clean up MPI
	MPI_Finalize();

	return rv;
}

int mainIO(int *params, int *mpi, long *data)
{
	FILE *fp;
	struct stat fbuf;
	int num_reads      = params[0];
	int num_writes     = params[1];
	int access_pattern = params[2];
	int num_accesses   = params[3];
	int io_min         = params[4];
	int io_max         = params[5];
	int stride_length  = params[6];
	int rank           = mpi[0];
	int nprocs         = mpi[1];

	// read and write buffers
	char *wbuf;
	char *rbuf = (char *) malloc(io_max);

	// for storing I/O request times
	long *timings;
	if (rank == 0) {
		timings = (long *) malloc(sizeof(long) * nprocs);
		memset(timings, 0, sizeof(long) * nprocs);
	}

	// open dump file
	if (!(fp = fopen("sonar-dump", "w+b"))) { // TODO: labios::
		std::cerr << "Failed to open/create dump file\n";
		return -1;
	}

	// perform read requests 'num_reads' times
	for (int read = 0; read < num_reads; read++) {
		for (int acc = 0; acc < num_accesses; acc++) {

			switch (access_pattern) {
				case RANDOM:{
					// get file size then seek to random offset
					if (stat("sonar-dump", &fbuf))
						fseek(fp, random(0, fbuf.st_size), SEEK_SET); // TODO: labios::
					break;
				}
				case STRIDED:{
					// set deliberate offset
					fseek(fp, stride_length, SEEK_CUR); // TODO: labios::
					break;
				}
			}

			auto start = Clock::now();
			auto rv = fread(rbuf, 1, random(io_min, io_max), fp); // TODO: labios::
			auto end = Clock::now();
			auto duration = std::chrono::duration_cast<Nanoseconds>(end-start).count();
		}
	}

	fseek(fp, 0, SEEK_SET); // TODO: labios::

	// perform write requests 'num_writes' times
	for (int read = 0; read < num_writes; read++) {
		for (int acc = 0; acc < num_accesses; acc++) {
			auto rsize = random(io_min, io_max);
			wbuf = generateRandomBuffer(rsize);

			switch (access_pattern) {
				case RANDOM:{
					// get file size then seek to random offset
					if (stat("sonar-dump", &fbuf))
						fseek(fp, random(0, fbuf.st_size), SEEK_SET); // TODO: labios::
					break;
				}
				case STRIDED:{
					// set deliberate offset
					fseek(fp, stride_length, SEEK_CUR); // TODO: labios::
					break;
				}
			}

			auto start = Clock::now();
			auto rv = fwrite(wbuf, 1, rsize, fp); // TODO: labios::
			auto end = Clock::now();
			auto duration = std::chrono::duration_cast<Nanoseconds>(end-start).count();

			// TODO: Gather then store

			free(wbuf);
		}
	}

	fclose(fp); // TODO: labios::
	free(rbuf);
	if (rank == 0)
		free(timings);

	return 0;
}

/*
 *	dumpRead - ~DEPRECATED~ - perform I/O dump phase for read tests
 */
int dumpRead(int *params, int *mpi, char *output_file, int compute_on)
{
	// int cols_per_row   = (num_phases * num_accesses * 2) + num_phases;
	// int cols_per_phase = (num_accesses * 2) + 1;

	// // allocate space for collected timing data
	// if (rank == 0) {
	// 	timings = (long *) malloc(sizeof(long) * num_procs);
	// 	data    = (long *) malloc(sizeof(long) * num_procs * cols_per_row);
	// }

	// std::cout << "Proc " << rank << ", Phase " << phase << ": " << duration << " ns\n";

	// // obtain processor timings
	// MPI_Gather(&duration, 1, MPI_LONG, timings, 1, MPI_LONG, 0, MPI_COMM_WORLD);

	// // log io access amount and duration
	// if (rank == 0) {
	// 	for (long proc = 0; proc < num_procs; proc++) {
	// 		/*
	// 			*   (proc * cols_per_row)           - offset to correct processor (or row)
	// 			*   (phase * cols_per_phase)        - offset to correct dump phase
	// 			*   (current_access * num_accesses) - offset to correct IO access
	// 			*/
	// 		data[(proc * cols_per_row) + (phase * cols_per_phase)] = phase;
	// 		if (rv) {
	// 			data[(proc * cols_per_row) + (phase * cols_per_phase) + (current_access * num_accesses) + 1] = rv;
	// 			data[(proc * cols_per_row) + (phase * cols_per_phase) + (current_access * num_accesses) + 2] = timings[proc];
	// 		}
	// 	}
	// }

	return 0;
}

/*
 *  logData - logs final dataset to .csv file
 */
int logData(long *data, int *params, int num_procs, char *output_file, int io_type)
{
	FILE *log;
	int num_phases   = params[0];
	int num_accesses = params[2];
	int first_write = 0;
	size_t rv;
	struct stat buf;
	std::string line = "";

	// check if output file exists
	if (stat(output_file, &buf))
		first_write = 1;

	// open log file (in append mode)
	if (!(log = fopen(output_file, "a+"))) {
		std::cerr << "Failed to open/create log file: " << output_file << "\n";
		return -1;
	}

	// add headers on first write only
	if (first_write) {
		line = "Processor, R/W";
		for (int i = 0; i < num_phases; i++) {
			line += ", Phase";
			for (int j = 0; j < num_accesses; j++) {
				line += ", IO Amount (KB), Time elapsed (ns)";
			}
		}
		line += "\n";
	}

	int num_cols = (num_phases * num_accesses * 2) + num_phases;
	for (int proc = 0; proc < num_procs; proc++) {
		// log => PROC, R/W
		line += std::to_string(proc) + ", " + std::to_string(io_type);

		for (long i = 0; i < num_cols; i++) {
			line += ", " + std::to_string(data[i]);
		}
		line += "\n";
	}

	// write to log
	if (!(rv = fwrite((char *) line.c_str(), line.length(), 1, log))) {
		std::cerr << "Failed to write to log\n";
		return -1;
	}

	fclose(log);

	return 0;
}

/*
 *	compute - performs compute phase of given intensity
 *		0 - no compute
 *		1 - busy sleep [default]
 *		2 - traditional arithmetic
 */
void compute(int intensity, int sleep_time)
{
	switch (intensity) {
		case NOCOMPUTE:
			break;
		case TRADITIONAL:{
			int size = 32;
			int A[size][size], B[size][size], C[size][size];

			// populate matrices with random integers
			for (int i = 0; i < size; i++) {
				for (int j = 0; j < size; j++) {
					A[i][j] = rand();
					B[i][j] = rand();
				}
			}

			// TODO: multiply them and store in third matrix

			break;
		}
		default:{
			// busy sleep
			auto start = Clock::now();
			auto end = Clock::now();
			auto elapsed = std::chrono::duration_cast<Seconds>(end-start).count();

			while (elapsed < sleep_time) {
				end = Clock::now();
				elapsed = std::chrono::duration_cast<Seconds>(end-start).count();
			}
			break;
		}
	}
	return;
}

/*
 *  random - generate a random, uniform number between MIN and MAX
 */
int random(int min, int max)
{
	std::random_device rand_dev;
	std::mt19937 gen(rand_dev());
	std::uniform_int_distribution<int> distr(min, max);
	return distr(gen);
}

/*
 *  generateRandomBuffer - populate buffer with random alphanumeric characters
 */
char* generateRandomBuffer(int size)
{
	// alphanumeric characters
	char characters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

	// allocate space for buffer
	char *rbuf = (char *) malloc(size);

	for (int chars = 0; chars < size; chars++) {
		// add random alphanumeric character a-zA-Z0-9
		rbuf[chars] = characters[random(0, sizeof(characters) - 1)];
	}

	return rbuf;
}

/*
 *  parseRequestSize - parses I/O range options (e.g., '16K' => 16 * 1024)
 */
int parseRequestSize(char *request)
{
	int num;
	std::string io(request);
	char *type = (char *) io.substr(io.length() - 1).c_str();

	if (type == (char *)'K') {
		num = std::atoi(io.substr(0, io.length() - 1).c_str()) * KB;
	} else if (type == (char *)'M') {
		num = std::atoi(io.substr(0, io.length() - 1).c_str()) * MB;
	} else {
		num = std::atoi(io.c_str());
	}

	return num;
}

/*
 *	showUsage - display Sonar options
 */
void showUsage(char** argv)
{
	std::cerr	<< "Usage: " << argv[0] << " [OPTIONS]\n"
				<< "Options:\n"
				<< "\t-h,\t\tPrint this help message\n"
				<< "\t-r,\t\tNumber of reads [3]\n"
				<< "\t-w,\t\tNumber of writes [1]\n"
				<< "\t-a,\t\tI/O Access pattern\n"
				<< "\t\t\t\t0 - [Sequential]\n"
				<< "\t\t\t\t1 - Random\n"
				<< "\t\t\t\t2 - Strided\n"
				<< "\t-i,\t\tNumber of I/O iterations [" << DEFAULT_PHASES << "]\n"
				<< "\t-R,\t\tNumber of I/O requests [" << DEFAULT_REQUESTS << "]\n"
				<< "\t-n,\t\tNumber of I/O accesses [" << DEFAULT_ACCESSES << "]\n"
				<< "\t-s,\t\tLower bound of I/O request (e.g., 4, 8K, or 16M) [" << MIN_IOSIZE << "K]\n"
				<< "\t-S,\t\tUpper bound of I/O request (e.g., 4, 8K, or 16M) [" << MIN_IOSIZE << "K]\n"
				<< "\t-l,\t\tStride length (in bytes) (access pattern=2) [" << DEFAULT_STRIDE << "B]\n"
				<< "\t-c,\t\tCompute intensity\n"
				<< "\t\t\t\t0 - None\n"
				<< "\t\t\t\t1 - [Busy Sleep]\n"
				<< "\t\t\t\t2 - Traditional\n"
				<< "\t-t,\t\tSleep time (compute intensity=1) [" << DEFAULT_SLEEP << "s]\n"
				<< "\t-o,\t\tOutput file logs to [sonar-log.txt]\n";
}


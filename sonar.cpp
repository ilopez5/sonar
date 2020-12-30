#include <unistd.h>
#include <sys/stat.h>
#include <chrono>
#include <mpi.h>
#include <random>
#include <iostream>
// #include <labios.h>
#include "sonar.h"

/* globals configurations */
int rank, nrows, nprocs;
int ncols             = 9;
int num_iterations    = DEFAULT_PHASES;            // 5 iterations
int num_requests      = DEFAULT_REQUESTS;          // 5 dumps
int num_accesses      = DEFAULT_ACCESSES;          // 1 access (per request)
int num_reads         = DEFAULT_READS;             // 3 reads
int num_writes        = DEFAULT_WRITES;            // 1 write
int stride_length     = DEFAULT_STRIDE;            // 10 byte stride
int sleep_time        = DEFAULT_SLEEP;             // 10 seconds
int matrix_size       = DEFAULT_MATRIX;            // 256 x 256
int access_pattern    = SEQUENTIAL;                // sequential access
int intensity         = BSLEEP;                    // busy sleep
int io_min            = MIN_IOSIZE * KB;           // 4 KB
int io_max            = MIN_IOSIZE * KB;           // 4 KB
char *output_file     = (char *)"./sonar-log.csv"; // default output file

/*
 *	main - driver for Sonar benchmark
 */
int main(int argc, char** argv)
{
	// parse program parameters
	parseArgs(argc, argv);
	
	// check if valid I/O range
	if (io_max < io_min) {
		std::cerr << "Invalid I/O range: " << io_min << "B - " << io_max << "B\n";
		return -1;
	}

	// initialize MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);   // obtain rank
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs); // obtain number of processes

	// allocate array for output dataset
	long *data;
	if (rank == 0)
		data = (long *) calloc(nrows * ncols, sizeof(long));

	// perform benchmark
	for (long i = 0; i < num_iterations; i++) {
		for (long r = 0; r < num_requests; r++) {
			if (mainIO(data, i, r) < 0) {
				free(data);
				return -1;
			}

			if (intensity)
				compute();
		}
		if (intensity)
			compute();
	}

	if (rank == 0) {
		logData(data);
		free(data);
	}

	MPI_Finalize();
	return 0;
}

/*
 *  mainIO - performs the I/O requests
 */
int mainIO(long *data, long iteration, long request)
{
	int io_size;
	struct stat fbuf;
	char *wbuf, *rbuf = (char *) malloc(io_max);

	// for storing each intermediate I/O request time
	long *timings;
	if (rank == 0)
		timings = (long *) calloc(nprocs, sizeof(long));

	// open dump file
	FILE *fp = fopen("sonar-dump", "w+b");
	if (!fp) {
		std::cerr << "Failed to open/create dump file\n";
		return -1;
	}

	// to track dump file size
	stat("sonar-dump", &fbuf);

	// perform write requests 'num_writes' times
	for (int write = 0; write < num_writes; write++) {
		io_size = random(io_min, io_max) / num_accesses;
		wbuf = (char *) malloc(io_size);

		for (int access = 0; access < num_accesses; access++) {
			// populate buffer with random alphanumeric characters
			generateRandomBuffer(wbuf, io_size);

			switch (access_pattern) {
				case RANDOM:{
					// seek to random offset % file size
					fseek(fp, random(0, fbuf.st_size), SEEK_SET);
					break;
				}
				case STRIDED:{
					// set deliberate offset
					fseek(fp, stride_length, SEEK_CUR);
					break;
				}
			}

			auto start    = Clock::now();
			auto rv       = fwrite(wbuf, 1, io_size, fp);
			auto end      = Clock::now();
			auto duration = std::chrono::duration_cast<Nanoseconds>(end-start).count();

			// gather then store timings
			MPI_Gather(&duration, 1, MPI_LONG, timings, 1, MPI_LONG, 0, MPI_COMM_WORLD);

			if (rank == 0) {
				int offset;
				for (long proc = 0; proc < nprocs; proc++) {
					// offset to correct row
					offset  = proc * (num_iterations * num_requests * (num_reads + num_writes) * num_accesses * ncols); // offset to processor block
					offset += iteration * (num_requests * (num_reads + num_writes) * num_accesses * ncols);             // offset to iteration block
					offset += request * ((num_reads + num_writes) * num_accesses * ncols);                              // offset to request block
					offset += num_reads * num_accesses * ncols;                                                         // offset past the read block
					offset += write * num_accesses * ncols;                                                             // offset to write
					offset += access * ncols;                                                                           // offset to access

					// store data
					data[offset]     = proc;
					data[offset + 1] = iteration;
					data[offset + 2] = request;
					data[offset + 5] = write;
					data[offset + 6] = access;
					data[offset + 7] = rv;
					data[offset + 8] = timings[proc];
				} // end for loop
			}
		} // end for loop

		free(wbuf);
	}

	// ensure read test has enough to go on
	int max_read = io_max * num_iterations * num_requests * num_reads * num_accesses;
	if (fbuf.st_size < max_read) {
		int diff = max_read - fbuf.st_size;
		wbuf = (char *) malloc(diff);
		generateRandomBuffer(wbuf, diff);
		fwrite(wbuf, 1, diff, fp);
		free(wbuf);
	}

	// reset offset
	fseek(fp, 0, SEEK_SET);

	// perform read requests 'num_reads' times
	for (long read = 0; read < num_reads; read++) {
		io_size = random(io_min, io_max) / num_accesses;

		for (long access = 0; access < num_accesses; access++) {
			switch (access_pattern) {
				case RANDOM:{
					// get file size then seek to random offset
					fseek(fp, random(0, fbuf.st_size), SEEK_SET);
					break;
				}
				case STRIDED:{
					// set deliberate offset
					fseek(fp, stride_length, SEEK_CUR);
					break;
				}
			}

			// perform IO and time it
			auto start    = Clock::now();
			auto rv       = fread(rbuf, 1, io_size, fp);
			auto end      = Clock::now();
			auto duration = std::chrono::duration_cast<Nanoseconds>(end-start).count();

			// gather results into timings array
			MPI_Gather(&duration, 1, MPI_LONG, timings, 1, MPI_LONG, 0, MPI_COMM_WORLD);

			if (rank == 0) {
				int offset;
				for (long proc = 0; proc < nprocs; proc++) {
					// offset to correct row
					offset  = proc * (num_iterations * num_requests * (num_reads + num_writes) * num_accesses * ncols); // offset to processor block
					offset += iteration * (num_requests * (num_reads + num_writes) * num_accesses * ncols);             // offset to iteration block
					offset += request * ((num_reads + num_writes) * num_accesses * ncols);                              // offset to request block
					offset += read * num_accesses * ncols;                                                              // offset to read
					offset += access * ncols;                                                                           // offset to access

					// store data
					data[offset]     = proc;
					data[offset + 1] = iteration;
					data[offset + 2] = request;
					data[offset + 3] = 1;
					data[offset + 4] = read;
					data[offset + 6] = access;
					data[offset + 7] = rv;
					data[offset + 8] = timings[proc];
				}
			}
		}
	}

	fclose(fp);
	free(rbuf);
	if (rank == 0)
		free(timings);

	return 0;
}

/*
 *  logData - logs final dataset to .csv file
 */
int logData(long *data)
{
	size_t rv;
	struct stat buf;
	std::string line = "";

	// add headers on first write only
	if (stat(output_file, &buf))
		line += "Processor, Iteration, Request, R/W, Read, Write, Access, Amount (B), Duration (ns)\n";

	// open log file (in append mode)
	FILE *log = fopen(output_file, "a+");
	if (!log) {
		std::cerr << "Failed to open/create log file: " << output_file << "\n";
		return -1;
	}

	// add dataset to c++ string
	for (int i = 0; i < nrows * ncols;) {
		line += std::to_string(data[i]);
		line += (++i % ncols) ? ", " : ",\n";
	}
	line += "\n";

	// write to log
	if (!(rv = fwrite((char *) line.c_str(), line.length(), 1, log)))
		std::cerr << "Failed to write to log\n";

	fclose(log);
	return rv;
}

/*
 *	compute - performs compute phase of given intensity
 *		0 - no compute
 *		1 - busy sleep [default]
 *		2 - traditional arithmetic
 */
void compute()
{
	switch (intensity) {
		case NOCOMPUTE:
			break;
		case TRADITIONAL:{
			int sum;
			int A[matrix_size][matrix_size];
			int B[matrix_size][matrix_size];
			int C[matrix_size][matrix_size];

			// populate matrices with random integers
			for (int i = 0; i < matrix_size; i++) {
				for (int j = 0; j < matrix_size; j++) {
					A[i][j] = random(0, 1000);
					B[i][j] = random(0, 1000);
				}
			}

			// multiply A*B and store in third matrix
			for (int i = 0; i < matrix_size; i++) {
				for (int j = 0; j < matrix_size; j++) {
					sum = 0;
					for (int k = 0; k < matrix_size; k++) {
						sum += A[i][k] * B[k][j];
					}
					C[i][j] = sum;
				}
			}
			break;
		}
		default:{
			// busy sleep
			auto start   = Clock::now();
			auto end     = Clock::now();
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
void generateRandomBuffer(char *rbuf, int size)
{
	// alphanumeric characters
	char chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

	// add random alphanumeric character a-zA-Z0-9
	for (int c = 0; c < size; c++)
		rbuf[c] = chars[random(0, sizeof(chars) - 1)];
}


/*
 *	parseArgs - accept and parse different configurations
 */
void parseArgs(int argc, char **argv)
{
	int opt, t;
	while ((opt = getopt(argc, argv, "h::r:w:x:i:R:n:l:o:s:S:e:t:m:")) != EOF) {
		switch (opt) {
			case 'h':
				showUsage(argv);
				exit(0);
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
				t = parseRequestSize(optarg);
				stride_length = (t >= MIN_IOSIZE && t <= MAX_IOSIZE) ? t : stride_length;
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
			case 'm':
				matrix_size = std::atoi(optarg);
				break;
			default:
				showUsage(argv);
				break;
		}
	} // end of while loop
	
	// update number of rows (for output log)
	nrows = nprocs * num_iterations * num_requests * (num_reads + num_writes) * num_accesses;
}

/*
 *  parseRequestSize - parses I/O range options (e.g., '16K' => 16 * 1024)
 */
int parseRequestSize(char *request)
{
	int num;
	std::string io(request);
	char *type = (char *) io.substr(io.length() - 1).c_str();

	if (type[0] == 'K')
		num = (std::atoi(io.substr(0, io.length() - 1).c_str())) * KB;
	else if (type[0] == 'M')
		num = std::atoi(io.substr(0, io.length() - 1).c_str()) * MB;
	else
		num = std::atoi(io.c_str());

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
				<< "\t-r,\t\tNumber of reads [" << DEFAULT_READS << "]\n"
				<< "\t-w,\t\tNumber of writes [" << DEFAULT_WRITES << "]\n"
				<< "\t-x,\t\tAccess pattern\n"
				<< "\t\t\t\t0 - [Sequential]\n"
				<< "\t\t\t\t1 - Random\n"
				<< "\t\t\t\t2 - Strided\n"
				<< "\t-i,\t\tNumber of I/O iterations [" << DEFAULT_PHASES << "]\n"
				<< "\t-R,\t\tNumber of I/O requests [" << DEFAULT_REQUESTS << "]\n"
				<< "\t-n,\t\tNumber of I/O accesses (per request) [" << DEFAULT_ACCESSES << "]\n"
				<< "\t-s,\t\tLower bound of I/O request size (e.g., 4, 8K, or 16M) [" << MIN_IOSIZE << "K]\n"
				<< "\t-S,\t\tUpper bound of I/O request size (e.g., 4, 8K, or 16M) [" << MIN_IOSIZE << "K]\n"
				<< "\t-l,\t\tStride length (access pattern=2) [" << DEFAULT_STRIDE << "B]\n"
				<< "\t-e,\t\tCompute intensity\n"
				<< "\t\t\t\t0 - None\n"
				<< "\t\t\t\t1 - [Busy Sleep]\n"
				<< "\t\t\t\t2 - Traditional\n"
				<< "\t-t,\t\tSleep time (intensity=1) [" << DEFAULT_SLEEP << "s]\n"
				<< "\t-m,\t\tSize of square matrix (intensity=2) [" << DEFAULT_MATRIX << "]\n"
				<< "\t-o,\t\tOutput file logs to [sonar-log.txt]\n";
}


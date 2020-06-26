#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <chrono>
#include <mpi.h>
#include "sonar.h"

/*
 *	main - driver for Sonar benchmark
 */
int main(int argc, char** argv)
{
	// benchmark parameters (w/ defaults)
	int rv, opt;
	int num_phases        = DEFAULT_PHASES;            // 5 dumps
	int workload_type     = WONLY;                     // write-only
	int access_pattern    = SEQUENTIAL;                // sequential access
	int stride_length     = DEFAULT_STRIDE;            // 10 byte stride
	int compute_intensity = SLEEP;                     // sleep-simulated
	int sleep_time        = DEFAULT_SLEEP;             // 10 seconds
	int io_size           = MIN_IOSIZE;                // 1 MB
	int num_accesses      = DEFAULT_NUM_ACCESS;        // 1 access per request
	char *output_file     = (char *)"./sonar-log.csv"; // default output file

	// check args
	if (argc == 1) {
		showUsage(argv);
		return 0;
	}

	// initialize MPI
	int rank, num_procs;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);      // obtain current process rank
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs); // obtain number of processes

	// parse given options
	while ((opt = getopt(argc, argv, "h::a:c:l:n:o:p:s:t:w:")) != EOF) {
		switch (opt) {
			case 'h':{
				showUsage(argv);
				return 0;
			}
			case 'a':{
				int tmpa = std::atoi(optarg);
				access_pattern = (tmpa >= MIN_ACCESS && tmpa <= MAX_ACCESS) ? tmpa : access_pattern;
				break;
			}
			case 'c':{
				int tmpc = std::atoi(optarg);
				compute_intensity = (tmpc <= MAX_INTENSITY && tmpc >= MIN_INTENSITY) ? tmpc : compute_intensity;
				break;
			}
			case 'l':{
				stride_length = std::atoi(optarg);
				break;
			}
			case 'n':{
				num_accesses = std::atoi(optarg);
				break;
			}
			case 'o':{
				output_file = optarg;
				break;
			}
			case 'p':{
				num_phases = std::atoi(optarg);
				break;
			}
			case 's':{
				int tmps = std::atoi(optarg);
				io_size = (tmps >= MIN_IOSIZE && tmps <= MAX_IOSIZE) ? tmps : io_size;
				break;
			}
			case 't':{
				sleep_time = std::atoi(optarg);
				break;
			}
			case 'w':{
				int tmpw = std::atoi(optarg);
				workload_type = (tmpw <= MAX_WORKLOAD && tmpw >= MIN_WORKLOAD) ? tmpw : workload_type;
				break;
			}
			default:{
				showUsage(argv);
				break;
			}
		}
	}
	
	// store parameters
	int params[] = 
		{ 
			num_phases,
			io_size,
			num_accesses,
			access_pattern,
			stride_length,
			compute_intensity, 
			sleep_time
		};

	// store mpi variables
	int mpi[] =
		{
			rank,
			num_procs
		};

	// run workload
	switch (workload_type) {
		case RONLY:{
			rv = dumpRead(params, mpi, output_file, 0);
			break;
		}
		case RCOMPUTE:{
			rv = dumpRead(params, mpi, output_file, 1);
			break;
		}
		case WONLY:{
			rv = dumpWrite(params, mpi, output_file, 0);
			break;
		}
		case WCOMPUTE:{
			rv = dumpWrite(params, mpi, output_file, 1);
			break;
		}
		case WRONLY:{
			rv = dumpWrite(params, mpi, output_file, 0);
			if (!rv)
				rv = dumpRead(params, mpi, output_file, 0);
			break;
		}
		case WRCOMPUTE:{
			rv = dumpWrite(params, mpi, output_file, 1);
			if (rv)
				rv = dumpRead(params, mpi, output_file, 1);
			break;
		}
		default:{
			rv = dumpWrite(params, mpi, output_file, 1);
			break;
		}
	}

	// clean up MPI
	MPI_Finalize();
	
	return rv;
}

/*
 *	dumpRead - perform I/O dump phase for read tests
 */
int dumpRead(int *params, int *mpi, char *output_file, int compute_on)
{
	// parameters
	int num_phases     = params[0];
	int io_size        = params[1];
	int num_accesses   = params[2];
	int access_pattern = params[3];
	int stride_length  = params[4];
	int intensity      = params[5];
	int sleep_time     = params[6];
	int size_access    = io_size / num_accesses;

	// mpi
	int rank      = mpi[0];
	int num_procs = mpi[1];

	// logging
	int rv, proc, row, col;
	int num_cols = num_phases << 1;
	int *data, *timings;
	
	if (rank == 0) {
		// allocate space for collected timing data
		timings = (int *) malloc(sizeof(int) * num_procs); // timing per dump
		data    = (int *) malloc(sizeof(int) * num_procs * num_phases * 2); // stores all
	}
	
	int phase = 0;
	while (phase < num_phases) {
		// start timer
		auto start = Clock::now();
		
		// perform read test
		switch (access_pattern) {
			case SEQUENTIAL:{
				break;
			}
			case RANDOM:{
				break;
			}
			case STRIDED:{
				break;
			}
		}

		// end timer
		auto end = Clock::now();
		int duration = std::chrono::duration_cast<Nanoseconds>(end-start).count();
		
		// gather timings 
		MPI_Gather(&duration, 1, MPI_INT, timings, 1, MPI_INT, 0, MPI_COMM_WORLD);
		
		// log space/time into dataset
		if (rank == 0) {
			for (proc = 0; proc < num_procs; proc++) {
				// store in dataset array
				row = num_cols * proc;
				col = phase << 1;
				data[row + col] = size_access;
				data[row + col + 1] = timings[proc];
			}
		}

		if (compute_on)
			compute(intensity, sleep_time);

		phase++;
	} // end while

	if (rank == 0)
		rv = logData(data, output_file, num_phases, num_procs, 0);
	
	return 0;
}


/*
 *	dumpWrite - perform I/O dump phase for write tests
 */
int dumpWrite(int *params, int *mpi, char *output_file, int compute_on)
{
	// parameters
	int num_phases     = params[0];
	int io_size        = params[1];
	int num_accesses   = params[2];
	int access_pattern = params[3];
	int stride_length  = params[4];
	int intensity      = params[5];
	int sleep_time     = params[6];
	int size_access    = io_size / num_accesses;

	// mpi
	int rank      = mpi[0];
	int num_procs = mpi[1];

	// logging
	int rv, proc, row, col;
	int num_cols = num_phases << 1;
	int *data, *timings;
	
	if (rank == 0) {
		// allocate space for collected timing data
		timings = (int *) malloc(sizeof(int) * num_procs); // timing per dump
		data = (int *) malloc(sizeof(int) * num_procs * num_phases * 2); // stores all
	}
	
	int phase = 0;
	while (phase < num_phases) {
		// start timer
		auto start = Clock::now();
		
		// perform write test
		switch (access_pattern) {
			case SEQUENTIAL:{
				break;
			}
			case RANDOM:{
				break;
			}
			case STRIDED:{
				break;
			}
		}

		// end timer
		auto end = Clock::now();
		int duration = std::chrono::duration_cast<Nanoseconds>(end-start).count();
		
		// gather timings 
		MPI_Gather(&duration, 1, MPI_INT, timings, 1, MPI_INT, 0, MPI_COMM_WORLD);
		
		// log space/time into dataset
		if (rank == 0) {
			for (proc = 0; proc < num_procs; proc++) {
				// store in dataset array
				row = num_cols * proc;
				col = phase << 1;
				data[row + col] = size_access;
				data[row + col + 1] = timings[proc];
			}
		}

		if (compute_on)
			compute(intensity, sleep_time);

		phase++;
	} // end while

	if (rank == 0)
		rv = logData(data, output_file, num_phases, num_procs, 1);
	
	return 0;
}

/*
 *	compute - performs compute phase of given intensity
 *		0 - no compute
 *		1 - sleep [default]
 *		2 - simple arithmetic
 *		3 - intense computations
 */
void compute(int intensity, int sleep_time)
{
	switch (intensity) {
		case NOCOMPUTE:{
			break;
		}
		case ARITHMETIC:{
			// do simple calculations
			break;
		}
		case INTENSE:{
			// do intense calculations
			break;
		}
		default:{
			// == SLEEP
			sleep(sleep_time);
			break;
		}
	}
	return;
}

/*
 *  logData - logs final dataset to .csv file
 */
int logData(int *data, char *output_file, int num_phases, int num_procs, int io_type)
{
	FILE *log;
	int rv, proc, row, col;
	int first_write = 0;
	int num_cols = num_phases << 1;
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
		for (proc = 0; proc < num_phases; proc++) {
			line += ", IO Amount (KB), Time elapsed (ns)";
		}
		line += "\n";
	}

	for (proc = 0; proc < num_procs; proc++) {
		// log data
		line += std::to_string(proc) + ", " + std::to_string(io_type);
		row = num_cols * proc; 
		for (col = 0; col < num_cols; col = col + 2) {
			// iterate through dumps
			line += ", " + std::to_string(data[row + col]);
			line += ", " + std::to_string(data[row + col + 1]);
		}
		line += "\n";
	}

	// write to log
	if (!(rv = fwrite(line.c_str(), line.length(), 1, log))) {
		std::cerr << "Failed to write to log\n";
		return -1;
	}

	fclose(log);
	
	return 0;
}

/*
 *	showUsage - display benchmark options
 */
void showUsage(char** argv)
{
	std::cerr	<< "Usage: " << argv[0] << " [OPTIONS]\n"
				<< "Options:\n"
				<< "\t-h,\t\tPrint this help message\n"
				<< "\t-p,\t\tNumber of I/O phases/dumps [" << DEFAULT_PHASES << "]\n"
				<< "\t-s,\t\tSize of I/O request (in KB) [" << MIN_IOSIZE << "KB]\n"
				<< "\t-n,\t\tNumber of I/O accesses [" << DEFAULT_NUM_ACCESS << "]\n"
				<< "\t-w,\t\tApplication workload type\n"
				<< "\t\t\t\t0 - Read only\n"
				<< "\t\t\t\t1 - [Write only]\n"
				<< "\t\t\t\t2 - Read with compute\n"
				<< "\t\t\t\t3 - Write with compute\n"
				<< "\t\t\t\t4 - Mixed w/o compute\n"
				<< "\t\t\t\t5 - Mixed with compute\n"
				<< "\t-a,\t\tI/O Access pattern\n"
				<< "\t\t\t\t0 - [Sequential]\n"
				<< "\t\t\t\t1 - Random\n"
				<< "\t\t\t\t2 - Strided\n"
				<< "\t-l,\t\tStride length (in Bytes) (access pattern=2) [" << DEFAULT_STRIDE << "B]\n"
				<< "\t-c,\t\tCompute intensity\n"
				<< "\t\t\t\t0 - None\n"
				<< "\t\t\t\t1 - [Sleep]\n"
				<< "\t\t\t\t2 - Arithmetic\n"
				<< "\t\t\t\t3 - Intense\n"
				<< "\t-t,\t\tSleep time (compute intensity=1) [" << DEFAULT_SLEEP << "s]\n"
				<< "\t-o,\t\tOutput file logs to [sonar-log.txt]\n";
}


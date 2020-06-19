#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
//#include <mpi.h>
#include "sonar.h"

/*
 *	main - primary driver for <unnamed> benchmark
 */
int main(int argc, char** argv)
{
	// vars/parameters with default values
	int rv, opt;
	int nprocs = MINPROCS;
	int nphases = 5;
	int workload_type = WONLY;
	int compute_intensity = SLEEP;
	int sleep_time = 10;
	char *output_file = NULL;

	// check args
	if (argc == 1) {
		show_usage(argv);
		return 0;
	}

	// initialize MPI
	//MPI_Init(&argc, &argv);

	while ((opt = getopt(argc, argv, "h:n:w:c:s:p:o:")) != EOF) {
		switch (opt) {
			case 'h':{
				show_usage(argv);
				break;
			}
			case 'n':{
				int tmpn = atoi(optarg);
				nprocs = (tmpn <= MAXPROCS && tmpn >= MINPROCS) ? tmpn : nprocs;
				break;
			}
			case 'w':{
				int tmpw = atoi(optarg);
				workload_type = (tmpw <= MAX_WORKLOAD && tmpw >= MIN_WORKLOAD) ? tmpw : workload_type;
				break;
			}
			case 'c':{
				int tmpc = atoi(optarg);
				compute_intensity = (tmpc <= MAX_INTENSITY && tmpc >= MIN_INTENSITY) ? tmpc : compute_intensity;
				break;
			}
			case 's':{
				sleep_time = atoi(optarg);
				break;
			}
			case 'p':{
				nphases = atoi(optarg);
				break;
			}
			case 'o':{
				output_file = optarg;
				break;
			}
			default:{
				show_usage(argv);
				break;
			}
		}
	}
	
	// store parameters
	int params[] = {nprocs, nphases, workload_type, compute_intensity, sleep_time};

	// run benchmark
	rv = benchmark(params, output_file);

	//MPI_Finalize();
	return 0;
}

int benchmark(int *params, char* output_file)
{
	// config vars
	int nprocs = params[0];
	int nphases = params[1];
	int workload_type = params[2];
	int compute_intensity = params[3];
	int sleep_time = params[4];
	int phase = 0;
	
	// run workloads
	while (phase < nphases) {
		// read
		if (!(workload_type % 2)) {
			// perform read
		}

		// write
		if (workload_type % 2) {
			// perform reads
		}

		// compute
		switch (compute_intensity) {
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
				sleep(sleep_time);
				break;
			}
		}
	}

	return 0;
}

/*
 *	show_usage - display program options
 */
void show_usage(char** argv)
{
	std::cerr	<< "Usage: " << argv[0] << " [OPTIONS]\n"
				<< "Options:\n"
				<< "\t-h,\t\tShow this help message\n"
				<< "\t-n,\t\tNumber of application processes ([1]-" << MAXPROCS << ")\n"
				<< "\t-p,\t\tNumber of I/O phases [5]\n"
				<< "\t-w,\t\tApplication workload type\n"
				<< "\t\t\t\t0 - Read only\n"
				<< "\t\t\t\t1 - [Write only]\n"
				<< "\t\t\t\t2 - Read with compute\n"
				<< "\t\t\t\t3 - Write with compute\n"
				<< "\t\t\t\t4 - Mixed w/o compute\n"
				<< "\t\t\t\t5 - Mixed with compute\n"
				<< "\t-c,\t\tCompute intensity\n"
				<< "\t\t\t\t0 - None\n"
				<< "\t\t\t\t1 - [Sleep]\n"
				<< "\t\t\t\t2 - Arithmetic\n"
				<< "\t\t\t\t3 - Intense\n"
				<< "\t-s,\t\tSleep time (compute intensity=1) [10s]\n"
				<< "\t-o,\t\tOutput file logs to [sonar-log.txt]\n";
}

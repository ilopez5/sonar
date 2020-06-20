#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <chrono>
//#include <mpi.h>
#include "sonar.h"

/*
 *	main - primary driver for Sonar benchmark
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
	char *output_file = (char *)"./sonar-log.txt";
	int io_size = 10;

	// check args
	if (argc == 1) {
		show_usage(argv);
		return 0;
	}

	// initialize MPI
	//MPI_Init(&argc, &argv);

	// parse given parameters
	while ((opt = getopt(argc, argv, "h::n:w:c:s:p:o:S:")) != EOF) {
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
			case 'S':{
				io_size = atoi(optarg);
				break;
			}
			default:{
				show_usage(argv);
				break;
			}
		}
	}
	
	// store parameters
	int params[] = 
		{
			nprocs, 
			nphases, 
			workload_type, 
			compute_intensity, 
			sleep_time, 
			io_size
		};

	// run workload
	switch (workload_type) {
		case RONLY:{
			read_dump(params, output_file, 0);
			break;
		}
		case RCOMPUTE:{
			read_dump(params, output_file, 1);
			break;
		}
		case WONLY:{
			write_dump(params, output_file, 0);
			break;
		}
		case WCOMPUTE:{
			write_dump(params, output_file, 1);
			break;
		}
		case WRONLY:{
			write_dump(params, output_file, 0);
			read_dump(params, output_file, 0);
			break;
		}
		case WRCOMPUTE:{
			write_dump(params, output_file, 1);
			read_dump(params, output_file, 1);
			break;
		}
		default:{
			write_dump(params, output_file, 1);
			break;
		}
	}

	//MPI_Finalize();
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
				<< "\t-o,\t\tOutput file logs to [sonar-log.txt]\n"
				<< "\t-S,\t\tSize of I/O request (in MB) [10]\n";
}

/*
 *	read_dump - perform I/O dump phase for read tests
 */
void read_dump(int *params, char* output_file, int compute_on)
{
	int nphases = params[1];
	int phase = 0;

	while (phase < nphases) {
		std::cout << "Dump Phase: " << phase << "\n";

		// start timer
		auto start = Clock::now();
		
		// TODO: READ STUFF

		// end timer
		auto end = Clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
		std::cout << "Time elapsed [R]: " << duration << " nanoseconds\n";

		if (compute_on)
			compute(params[3], params[4]);

		// step to next iteration
		phase++;
	} // end of while loop
	
	return;
}

/*
 *	write_dump - perform I/O dump phase for write tests
 */
void write_dump(int *params, char* output_file, int compute_on)
{
	int nphases = params[1];
	int phase = 0;

	while (phase < nphases) {
		std::cout << "Dump Phase: " << phase << "\n";

		// start timer
		auto start = Clock::now();

		// TODO: WRITE STUFF
			/*		
			// open/create dump file
			if (!(fp = fopen(dump_file, "w+"))) {
				std::cerr << "Failed to open/create dump file: " << output_file << "\n";
				return -1;
			}

			// TODO: randomly generate buffer contents
			io_buffer = generateRandomBuffer(io_size);

			// write to file
			if (!(rv = fwrite(io_buffer, io_size, 1, fp))) {
				std::cerr << "Failed to write to: " << output_file << "\n";
				return -1;
			}
			*/

		// io_buffer = generateRandomBuffer(io_size);
			/*
			// open log file
			if (!(log = fopen(output_file, "w+"))) {
				std::cerr << "Failed to open/create log file\n";
				return -1;
			}

			// write to log file
			if (!(rv = fwrite(log_buffer, sizeof(log_buffer), 1, log))) {
				std::cerr << "Failed to log timings\n";
				return -1;
			}

			fclose(log);
			*/


		// end timer
		auto end = Clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
		std::cout << "Time elapsed [W]: " << duration << " nanoseconds\n";

		if (compute_on)
			compute(params[3], params[4]);

		// step to next iteration
		phase++;
	} // end of while loop

	return;
}

/*
 *	compute - performs compute phase of given intensity
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

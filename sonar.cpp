#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <chrono>
#include <mpi.h>
#include "sonar.h"

/*
 *	main - driver for Sonar benchmark
 */
int main(int argc, char** argv)
{
	// parameters with default values
	int rv, opt;
	int num_phases = DEFAULT_PHASES;               // 5 dumps
	int workload_type = WONLY;                     // write-only
	int access_pattern = SEQUENTIAL;               // sequential access
	int stride_length = DEFAULT_STRIDE;            // 10 byte stride
	int compute_intensity = SLEEP;                 // sleep-simulated
	int sleep_time = DEFAULT_SLEEP;                // 10 seconds
	char *output_file = (char *)"./sonar-log.txt"; // default output file
	int io_size = DEFAULT_IOSIZE;                  // 1 MB

	// check args
	if (argc == 1) {
		show_usage(argv);
		return 0;
	}

	// initialize MPI
	MPI_Init(&argc, &argv);

	// parse given parameters
	while ((opt = getopt(argc, argv, "h::n:s:w:a:l:c:t:o:")) != EOF) {
		switch (opt) {
			case 'h':{
				show_usage(argv);
				return 0;
			}
			case 'n':{
				num_phases = atoi(optarg);
				break;
			}
			case 's':{
				io_size = atoi(optarg);
				break;
			}
			case 'w':{
				int tmpw = atoi(optarg);
				workload_type = (tmpw <= MAX_WORKLOAD && tmpw >= MIN_WORKLOAD) ? tmpw : workload_type;
				break;
			}
			case 'a':{
				int tmpa = atoi(optarg);
				access_pattern = (tmpa >= MIN_ACCESS && tmpa <= MAX_ACCESS) ? tmpa : access_pattern;
				break;
			}
			case 'l':{
				stride_length = atoi(optarg);
				break;
			}
			case 'c':{
				int tmpc = atoi(optarg);
				compute_intensity = (tmpc <= MAX_INTENSITY && tmpc >= MIN_INTENSITY) ? tmpc : compute_intensity;
				break;
			}
			case 't':{
				sleep_time = atoi(optarg);
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
	int params[] = 
		{ 
			num_phases, 
			workload_type,
			compute_intensity, 
			sleep_time, 
			io_size,
			access_pattern,
			stride_length
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

	MPI_Finalize();
	return 0;
}

/*
 *	show_usage - display benchmark options
 */
void show_usage(char** argv)
{
	std::cerr	<< "Usage: " << argv[0] << " [OPTIONS]\n"
				<< "Options:\n"
				<< "\t-h,\t\tShow this help message\n"
				<< "\t-n,\t\tNumber of I/O phases [" << DEFAULT_PHASES << " dumps]\n"
				<< "\t-s,\t\tSize of I/O request (in MB) [" << DEFAULT_IOSIZE << "MB]\n"
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

/*
 *	read_dump - perform I/O dump phase for read tests
 */
void read_dump(int *params, char* output_file, int compute_on)
{
	int num_phases = params[1];
	int phase = 0;

	while (phase < num_phases) {
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
	int num_phases = params[1];
	int phase = 0;

	while (phase < num_phases) {
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
			// params[3] => intensity, params[4] => sleep duration
			compute(params[3], params[4]);

		// step to next iteration
		phase++;
	} // end of while loop

	return;
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

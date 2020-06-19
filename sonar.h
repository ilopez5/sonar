/* macros */
#define MAXPROCS 32
#define MINPROCS 1
#define MAX_WORKLOAD 5
#define MIN_WORKLOAD 0
#define MAX_INTENSITY 3
#define MIN_INTENSITY 0

// defines workload types
#define RONLY 0
#define WONLY 1
#define RCOMPUTE 2
#define WCOMPUTE 3
#define WRONLY 4
#define WRCOMPUTE 5

// defines compute types
#define NOCOMPUTE 0
#define SLEEP 1
#define ARITHMETIC 2
#define INTENSE 3

/* function prototypes */
void show_usage(char **);
int benchmark(int*, char*);

/* typedefs */
typedef std::chrono::high_resolution_clock Clock;

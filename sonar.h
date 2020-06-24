/* macros */
#define MIN_WORKLOAD 0
#define MAX_WORKLOAD 5
#define MIN_INTENSITY 0
#define MAX_INTENSITY 3
#define MIN_ACCESS 0
#define MAX_ACCESS 2

// defines workload types
#define RONLY 0
#define WONLY 1
#define RCOMPUTE 2
#define WCOMPUTE 3
#define WRONLY 4
#define WRCOMPUTE 5

// defines access patterns
#define SEQUENTIAL 0
#define RANDOM 1
#define STRIDED 2

// defines compute types
#define NOCOMPUTE 0
#define SLEEP 1
#define ARITHMETIC 2
#define INTENSE 3

// defaults
#define DEFAULT_PHASES 5   // dumps
#define DEFAULT_STRIDE 10  // bytes
#define DEFAULT_SLEEP 10   // seconds
#define DEFAULT_IOSIZE 1   // MB

/* function prototypes */
void show_usage(char**);
void read_dump(int*,char*,int);
void write_dump(int*,char*,int);
void compute(int,int);

/* typedefs */
typedef std::chrono::high_resolution_clock Clock;

/* macros */
#define MIN_WORKLOAD 0
#define MAX_WORKLOAD 5
#define MIN_INTENSITY 0
#define MAX_INTENSITY 3
#define MIN_ACCESS 0
#define MAX_ACCESS 2
#define MIN_IOSIZE 4    // KB
#define MAX_IOSIZE 8000 // 8MB

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
#define DEFAULT_PHASES 5      // dumps
#define DEFAULT_STRIDE 10     // bytes
#define DEFAULT_SLEEP 10      // seconds
#define DEFAULT_NUM_ACCESS 1  // I/O request splits

/* function prototypes */
void showUsage(char**);
void compute(int,int);
int logData(int*,char*,int,int,int);
int dumpRead(int*,int*,char*,int);
int dumpWrite(int*,int*,char*,int);

/* typedefs */
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::nanoseconds Nanoseconds;

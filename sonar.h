/* macros */
#define MIN_INTENSITY 0
#define MAX_INTENSITY 2
#define MIN_ACCESS 0
#define MAX_ACCESS 2
#define MIN_IOSIZE 4     // 4B
#define MAX_IOSIZE 16384 // 16MB
#define KB 1024
#define MB (KB * KB)

// defines access patterns
#define SEQUENTIAL 0
#define RANDOM 1
#define STRIDED 2

// defines compute types
#define NOCOMPUTE 0
#define BSLEEP 1
#define TRADITIONAL 2

// defaults
#define DEFAULT_PHASES 5      // # iterations
#define DEFAULT_REQUESTS 5    // # requests
#define DEFAULT_ACCESSES 1    // # accesses (per request)
#define DEFAULT_READS 3       // # reads
#define DEFAULT_WRITES 1      // # writes
#define DEFAULT_STRIDE 10     // bytes
#define DEFAULT_SLEEP 5       // seconds
#define DEFAULT_MATRIX 256



/* function prototypes */
int mainIO(int*,long*,long,long);
int logData(long*,int*,char*);
int random(int,int);
int parseRequestSize(char*);
void showUsage(char**);
void compute(int,int,int);
char* generateRandomBuffer(int);

/* typedefs */
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::nanoseconds Nanoseconds;
typedef std::chrono::seconds Seconds;

int dumpRead(int*,int*,char*,int);
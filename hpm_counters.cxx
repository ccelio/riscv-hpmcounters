#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <vector>
#include <array>

// In setStats, we might trap reading uarch-specific counters.
// The trap handler will skip over the instruction and write 0,
// but only if a0 is the destination register.
#define read_csr_safe(reg) ({ register long __tmp asm("a0"); \
  asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
  __tmp; })

// 100e6 cycles
#define SLEEP_TIME_US (100000)

// How many counters do we support? (change for your micro-architecture).
#define NUM_COUNTERS (8)
//#define NUM_COUNTERS (32) maximum amount of HPMs is 32
typedef std::array<long, NUM_COUNTERS> snapshot_t;

static char const*               counter_names[NUM_COUNTERS];
static snapshot_t                init_counters;
static std::vector <snapshot_t>  counters;


// use sprintf to attempt to lesson load on debug interface
#define CHAR_PER_LINE (40)
#define MAX_BUFFER_SZ (CHAR_PER_LINE * NUM_COUNTERS)

enum StatState
{
   INIT,   // initialize the counters
   WAKEUP, // print the running diff from last snapshot
   FINISH, // print the total CPI
   MAX
};

int bytes_added(int result)
{
   if (result > 0) {
      return result;
   } else {
      printf ("Error in sprintf. Res: %d\n", result);
      return 0;
   }
}

#if 1
static int handle_stats(int enable)
{
   long tsc_start = read_csr_safe(cycle);
   long irt_start = read_csr_safe(instret);

   sigset_t sig_set;
   sigemptyset(&sig_set);
   sigaddset(&sig_set, SIGTERM);

   if (sigprocmask(SIG_BLOCK, &sig_set, NULL) < 0) {
      perror ("sigprocmask failed");
      return 1;
   }

   static size_t step = 0; // increment every time handle_stats is called

   int i = 0;         
   snapshot_t snapshot;
#define READ_CTR(name) do { \
      if (i < NUM_COUNTERS) { \
         long csr = read_csr_safe(name); \
         if (enable == INIT)   { init_counters[i] = csr; snapshot[i] = 0; counter_names[i] = #name; } \
         if (enable == WAKEUP) { snapshot[i] = csr - init_counters[i]; } \
         if (enable == FINISH) { snapshot[i] = csr - init_counters[i]; } \
         i++; \
      } \
   } while (0)

   // Since most processors will not support all 32 HPMs, comment out which hpm counters you don't want to track.
   READ_CTR(cycle);
   READ_CTR(instret);
   READ_CTR(time);
   READ_CTR(hpmcounter3);
   READ_CTR(hpmcounter4);
   READ_CTR(hpmcounter5);
   READ_CTR(hpmcounter6);
   READ_CTR(hpmcounter7);
   READ_CTR(hpmcounter8);
   READ_CTR(hpmcounter9);
   READ_CTR(hpmcounter10);
   READ_CTR(hpmcounter11);
   READ_CTR(hpmcounter12);
   READ_CTR(hpmcounter13);
   READ_CTR(hpmcounter14);
   READ_CTR(hpmcounter15);
   READ_CTR(hpmcounter16);
   READ_CTR(hpmcounter17);
   READ_CTR(hpmcounter18);
   READ_CTR(hpmcounter19);
   READ_CTR(hpmcounter20);
   READ_CTR(hpmcounter21);
   READ_CTR(hpmcounter22);
   READ_CTR(hpmcounter23);
   READ_CTR(hpmcounter24);
   READ_CTR(hpmcounter25);
   READ_CTR(hpmcounter26);
   READ_CTR(hpmcounter27);
   READ_CTR(hpmcounter28);
   READ_CTR(hpmcounter29);
   READ_CTR(hpmcounter30);
   READ_CTR(hpmcounter31);

   counters.push_back(snapshot);

   //printf("Snapshot Time in cycles : %ld\n", read_csr_safe(cycle) - tsc_start);
   //printf("Snapshot Time in instret: %ld\n", read_csr_safe(instret) - irt_start);
   //if (step % 10 == 0) printf("heartbeat: %d\n", step);
   step++;

#undef READ_CTR
   if (enable == FINISH || step % 30 == 0) { 
      for (auto & element : counters) {
         for (int i = 0; i < NUM_COUNTERS; i++) {
            long c = element[i];
            if (c) {
               printf("##  %s = %ld\n", counter_names[i], c);
            }
         }
      }
      if (enable != FINISH) counters.clear();

      //printf("Print Time in cycles : %ld\n", read_csr_safe(cycle) - tsc_start);
      //printf("Print Time in instret: %ld\n", read_csr_safe(instret) - irt_start);
   }

   if (sigprocmask(SIG_UNBLOCK, &sig_set, NULL) < 0) {
      perror ("sigprocmask unblock failed");
      return 1;
   }

   return 0;
}
#else
static int handle_stats(int enable) { return 0; }
#endif
 
void sig_handler(int signum)
{
   handle_stats(FINISH);
   //printf("HPM Counters exiting, received handler: %d\n", signum);
   exit(0);
}

int main(int argc, char** argv)
{
   signal(SIGINT, sig_handler);
   signal(SIGTERM, sig_handler);

   if (argc > 1)
   {
      // only print the final cycle and instret counts.
      //printf ("Pausing, argc=%d\n", argc);
      handle_stats(INIT);
      pause();
   }
   else
   {
      //printf("Starting: counter array size: %d\n", sizeof(counters));
      handle_stats(INIT);
      while (1)
      {
         usleep(SLEEP_TIME_US);
         handle_stats(WAKEUP);
      }
      //printf("Exiting\n");
   }

   return 0;
}

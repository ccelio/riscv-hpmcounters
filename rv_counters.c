#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

// In setStats, we might trap reading uarch-specific counters.
// The trap handler will skip over the instruction and write 0,
// but only if a0 is the destination register.
#define read_csr_safe(reg) ({ register long __tmp asm("a0"); \
  asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
  __tmp; })

#define MAX_SNAPSHOTS (2)
#define NUM_COUNTERS (48)
static char* counter_names[NUM_COUNTERS];
static long init_counters [NUM_COUNTERS]; // value of counter at start
static long counters      [NUM_COUNTERS*MAX_SNAPSHOTS];
static size_t step = 0; // increment every time handle_stats is called

// use sprintf to lesson load on debug interface
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

// mark true when inside the sig_handler - set to false once finished.
// prevents us double-printing data.
static sig_atomic_t lock = 0;

#if 1
static int handle_stats(int enable)
{
   sigset_t sig_set;
   sigemptyset(&sig_set);
   sigaddset(&sig_set, SIGTERM);

   if (sigprocmask(SIG_BLOCK, &sig_set, NULL) < 0) {
      perror ("sigprocmask failed");
      return 1;
   }

   if (lock)
      printf("in_function true! Are we terminating while trying to handle wakeup stats?\n");
   lock = 1;

   int i = 0;         
#define READ_CTR(name) do { \
      while (i >= NUM_COUNTERS) ; \
      long csr = read_csr_safe(name); \
      if (enable == INIT)   { init_counters[i] = csr; counters[i] = csr; counter_names[i] = #name; } \
      if (enable == WAKEUP) { counters[i + (step*NUM_COUNTERS)] = csr - init_counters[i]; } \
      if (enable == FINISH) { counters[i + (step*NUM_COUNTERS)] = csr - init_counters[i]; } \
      i++; \
   } while (0)
   READ_CTR(cycle);
   READ_CTR(instret);
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

   step++;
   if (step % 10 == 0) printf("step: %d\n", step);

#undef READ_CTR
   printf ("enable: %d, step: %d\n", enable, step);
   if (enable == FINISH || (enable == WAKEUP && (step) == MAX_SNAPSHOTS)) {
      for (int s = 0; s < step; s++) {
         char buffer [MAX_BUFFER_SZ];
         int length = 0;
         for (int x = 0; x < NUM_COUNTERS; x++) {
            long c = counters[x + (s*NUM_COUNTERS)];
            if (enable == FINISH && s == (step-1) && c) {
               length += bytes_added(sprintf(buffer+length, "##@@ %s = %ld (total)\n", counter_names[x], c));
            }
            else if (c) {
               length += bytes_added(sprintf(buffer+length, "##  %s = %ld\n", counter_names[x], c));
            }
//            printf("length: %d\n");
         }
         printf(buffer);
//         printf("length: %d\n");
      }
   }

   step = step % MAX_SNAPSHOTS;
   printf("Leaving Function.----");
   lock = 0;
   printf("Leaving Function2.\n");
   
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
   printf("RV Counters exiting, received handler: %d\n", signum);
   exit(0);
}

int main(int argc, char** argv)
{
   signal(SIGINT, sig_handler);
   signal(SIGTERM, sig_handler);

   if (argc > 1)
   {
      // only print the final cycle and instret counts.
      printf ("Pausing, argc=%d\n", argc);
      handle_stats(INIT);
      pause();
   }
   else
   {
      printf("Starting: counter array size: %d\n", sizeof(counters));
      handle_stats(INIT);
      while (1)
      {
         usleep(100000);
         handle_stats(WAKEUP);
      }
      printf("Exiting\n");
   }

   return 0;
}

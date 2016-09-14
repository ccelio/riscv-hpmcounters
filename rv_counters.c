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
  
//#define NUM_COUNTERS (32)
#define NUM_COUNTERS (48)
static long init_counters[NUM_COUNTERS]; // value of counter at start
static long counters[NUM_COUNTERS];
static char* counter_names[NUM_COUNTERS];
#define INSTRET_ID (1)

enum StatState
{
   INIT,   // initialize the counters
   WAKEUP, // print the running diff from last snapshot
   FINISH, // print the total CPI
   MAX
};

#if 1
static int handle_stats(int enable)
{
   int i = 0;
#define READ_CTR(name) do { \
      while (i >= NUM_COUNTERS) ; \
      long csr = read_csr_safe(name); \
      if (enable == INIT)   { init_counters[i] = csr; counters[i] = csr; counter_names[i] = #name; } \
      if (enable == WAKEUP) { counters[i] = csr - init_counters[i]; } \
      if (enable == FINISH) { counters[i] = csr - init_counters[i]; } \
      i++; \
   } while (0)
   READ_CTR(cycle);   READ_CTR(instret);
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
#if 0
   READ_CTR(0xcc0);  READ_CTR(0xcc1);  READ_CTR(0xcc2);  READ_CTR(0xcc3);
   READ_CTR(0xcc4);  READ_CTR(0xcc5);  READ_CTR(0xcc6);  READ_CTR(0xcc7);
   READ_CTR(0xcc8);  READ_CTR(0xcc9);  READ_CTR(0xcca); READ_CTR(0xccb);
   READ_CTR(0xccc); READ_CTR(0xccd); READ_CTR(0xcce); READ_CTR(0xccf);
//   READ_CTR(uarch0);  READ_CTR(uarch1);  READ_CTR(uarch2);  READ_CTR(uarch3);
//   READ_CTR(uarch4);  READ_CTR(uarch5);  READ_CTR(uarch6);  READ_CTR(uarch7);
//   READ_CTR(uarch8);  READ_CTR(uarch9);  READ_CTR(uarch10); READ_CTR(uarch11);
//   READ_CTR(uarch12); READ_CTR(uarch13); READ_CTR(uarch14); READ_CTR(uarch15);
#endif

#undef READ_CTR
   if (enable == FINISH) {
      for (int x = 0; x < NUM_COUNTERS; x++) {
         if (counters[x]) {
            printf("##@@ %s = %ld (total)\n", counter_names[x], counters[x]);
         }
      }
   }
   else if (enable == WAKEUP) {
      for (int x = 0; x < NUM_COUNTERS; x++) {
         if (counters[x]) {
            printf("##  %s = %ld\n", counter_names[x], counters[x]);
         }
      }
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
      printf("Starting\n");
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

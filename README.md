This utility helps monitor performance of RISC-V processors running Linux.

RISC-V processors implement Hardware Performance Monitor (HPM) counters.

So long as these counters are unprotected and not saved and restored on context
switches, other Process A can read the HPM counters to monitor the performance
of Process B. 

`hpm_counters` is a binary that can run in Linux and checkpoint the HPM counters
every (roughly) 100M cycles (you can set the sleep time). When `hpm_counters` is
sent a SIGKILL, it will print out all of its checkpoints.

*Requirements*

You must install the glib RISC-V gcc compiler (`riscv64-unknown-linux-gnu-g++`).

You must modify the machine initialization in your boot process to initialize
the HPM counters are desired. You must also give user-access privileges to the
HPM counters.


*How to Use?*

````
    $ ./hpm_counters &
    $ ./mybenchmark
    $ killall hpm_counters
    $ while pgrep hpm_counters > /dev/null; do sleep 1; done
````


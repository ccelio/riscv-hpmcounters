
CC=riscv64-unknown-linux-gnu-g++
#CC=riscv64-unknown-linux-gnu-gcc
#CC=gcc -std=c99
CFLAGS=-static -O2
LDFLAGS=

rv_counters: rv_counters.c Makefile
	$(CC) $(CFLAGS) -o $@ $@.c
	riscv-objdump --disassemble-all $@ > $@.dump
	cp $@ /nscratch/celio/initram/rv_counters_hpm
 
run: rv_counters
	spike pk ./rv_counters
#	./rv_counters

clean:
	rm -f rv_counters *.o

.PHONY: run clean

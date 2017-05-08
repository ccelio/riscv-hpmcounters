
CC=riscv64-unknown-linux-gnu-g++
CFLAGS=-O2
#CFLAGS=-static -O2
#CFLAGS=-static -O2 -Wno-string
LDFLAGS=

rv_counters: rv_counters.c Makefile
	$(CC) $(CFLAGS) -o $@ $@.c
	cp $@ /nscratch/celio/initram/rv_counters_hpm
#	riscv-objdump --disassemble-all $@ > $@.dump
 
run: rv_counters
	spike pk ./rv_counters

clean:
	rm -f rv_counters *.o

.PHONY: run clean

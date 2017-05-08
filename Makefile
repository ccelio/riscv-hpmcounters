
CC=riscv64-unknown-linux-gnu-g++
CFLAGS=-O2
LDFLAGS=

hpm_counters: hpm_counters.c Makefile
	$(CC) $(CFLAGS) -o $@ $@.c
	riscv-objdump --disassemble-all $@ > $@.dump

# likely won't work, as we're compiled using glibc
run: hpm_counters
	spike pk ./hpm_counters

clean:
	rm -f hpm_counters *.o *.dump

.PHONY: run clean

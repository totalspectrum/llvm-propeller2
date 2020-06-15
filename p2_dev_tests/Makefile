LLVM = ~/Github/llvm-propeller2/build
CC = $(LLVM)/bin/clang
LD = $(LLVM)/bin/ld.lld
LOAD = loadp2

TARGET = test7
OBJDIR = build

CFLAGS = -ffunction-sections --target=p2 -Os
LFLAGS = -Tp2.ld
LOADFLAGS = -v -ZERO

.PHONY: all setup clean load

all: clean setup $(OBJDIR)/$(TARGET).elf load

setup:
	mkdir -p $(OBJDIR)

$(OBJDIR)/$(TARGET).elf: $(OBJDIR)/$(TARGET).o $(OBJDIR)/propeller2.o $(OBJDIR)/crt0.o
		$(LD) $(LFLAGS) -o $@ $^

$(OBJDIR)/$(TARGET).o: $(TARGET).cpp propeller2.h
		$(CC) $(CFLAGS) -o $@ -c $< #-mllvm -debug #--debug-only=p2-isel,isel,p2-expand-pseudos,mccodeemitter

$(OBJDIR)/propeller2.o: propeller2.c propeller2.h
		$(CC) $(CFLAGS) -o $@ -c $<

$(OBJDIR)/crt0.o: crt0.c
		$(CC) $(CFLAGS) -o $@ -c $< -mllvm --debug-only=p2-asm-parser

load:
	$(LOAD) $(LOADFLAGS) $(OBJDIR)/$(TARGET).elf

clean:
	rm -rf ./$(OBJDIR)
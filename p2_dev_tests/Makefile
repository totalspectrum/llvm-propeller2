LLVM = ~/Github/llvm-propeller2/build
CC = $(LLVM)/bin/clang
LD = $(LLVM)/bin/ld.lld
LOAD = loadp2

TARGET = test10
OBJDIR = build

CFLAGS = -O1 -ffunction-sections -fdata-sections --target=p2 -I./p2lib
LFLAGS = -Tp2.ld
LOADFLAGS = -v -ZERO -b 230400 -t

.PHONY: all setup clean load

all: clean setup $(OBJDIR)/$(TARGET).elf load

setup:
	$(shell pkill loadp2)
	mkdir -p $(OBJDIR)

$(OBJDIR)/$(TARGET).elf: $(OBJDIR)/$(TARGET).o $(OBJDIR)/propeller2.o $(OBJDIR)/crt0.o
		$(LD) $(LFLAGS) -o $@ $^

$(OBJDIR)/$(TARGET).o: $(TARGET).cpp p2lib/propeller2.h
		$(CC) $(CFLAGS) -o $@ -c $< #-mllvm --debug-only=p2-isel-lower,p2-frame-lower

$(OBJDIR)/propeller2.o: p2lib/propeller2.c p2lib/propeller2.h
		$(CC) $(CFLAGS) -o $@ -c $< #-mllvm --debug-only=p2-frame-lower

$(OBJDIR)/crt0.o: p2lib/crt0.c
		$(CC) $(CFLAGS) -o $@ -c $< #-mllvm -debug

load:
	$(LOAD) $(LOADFLAGS) $(OBJDIR)/$(TARGET).elf

clean:
	rm -rf ./$(OBJDIR)
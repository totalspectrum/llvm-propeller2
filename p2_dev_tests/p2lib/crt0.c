// due to current limitations of the ASM parser, need to manually calculate jump offsets.
// eventually need label support so that jumps can be done much more easily

#include "propeller2.h"

void _entry() {
    // basic entry code to jump to our resuable startup code.
    // we inline this function, and it will get overwritten later by hub params (clkfreq, clkmode, etc)
    // jump relative to address 0x100. -4 because of how jmp instruction works
    // the linker will place _start() at address 0x100
    asm("jmp #252");
}

void _start() {
    asm("cogid $r0\n"           // get the current cog ID
        "tjz $r0, #5\n"         // if cog 0, jump to the special cog0 startup code.

        "mov $r0, $ptra\n"      //  if not cog 0, save ptra (value at ptra should be pointer to start of stack)
        "rdlong $r1, $r0\n"     //  read out first stack value
        "add $r0, #4\n"
        "rdlong $r0, $r0\n"     //  read out second stack value
        "jmp $r1\n"             //  jump to the cog function

        "augs #1\n"
        "mov $ptra, #0\n"       // cog0 startup: start the stack at 0x200 to reserve room for startup code/global things stored at the start of memory
        "augs #2\n"
        "mov $r0, #0\n"         // r0 = 0x400
        "jmp $r0");             // jump to the start of our program (0x400)
}
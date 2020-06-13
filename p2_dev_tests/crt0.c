// due to current limitations of the ASM parser, need to manually calculate jump offsets.
// eventually need label support so that jumps can be done much more easily

void _start() {
    asm("cogid $r0\n"           // get the current cog ID
        "tjz $r0, #5\n"         // if cog 0, jump to the special cog0 startup code.

        "mov $sp, $ptra\n"      //  if not cog 0, set sp to ptra (value at ptra should be pointer to start of stack)
        "rdlong $r1, $ptra\n"   //  read out first stack value
        "add $ptra, #4\n"
        "rdlong $r0, $ptra\n"   //  read out second stack value
        "jmp $r1\n"             //  jump to the cog function

        "augs #1\n"
        "mov $sp, #0\n"         // cog0 startup: start the stack at 0x200 to reserve room for startup code/cached functions
        "augs #2\n"
        "mov $r0, #0\n"         // r0 = 0x400
        "jmp $r0");             // jump to the start of our program (0x400)
}
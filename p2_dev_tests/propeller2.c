#include "propeller2.h"

extern void _start();

void coginit(int mode, void (*f)(void *), int par, int *stack) {
    stack[0] = (int)f;
    stack[1] = par;
    asm("setq %0\n"
        "coginit %1, %2"
        : // no outputs
        : "r"(stack), "r"(mode), "r"(_start)
        );
}

void cognew(void (*f)(void *), int par, int *stack) {
    coginit(0x10, f, par, stack);
}
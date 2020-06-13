#ifndef _PROPELLER2_H
#define _PROPELLER2_H

#define waitx(t) asm("waitx %0" : : "r"(t))
#define dirh(pin) asm("dirh %0" : : "r"(pin))
#define dirl(pin) asm("dirl %0" : : "r"(pin))
#define outh(pin) asm("outh %0" : : "r"(pin))
#define outl(pin) asm("outl %0" : : "r"(pin))

#ifdef __cplusplus
extern "C" {
#endif

void coginit(int mode, void (*f)(void *), int par, int *stack);
void cognew(void (*f)(void *), int par, int *stack);

#ifdef __cplusplus
}
#endif

#endif
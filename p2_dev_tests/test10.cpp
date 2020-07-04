/*
  This tests variadic functions. One of the last things needed before we can begin implementing a much more complex library
  Also, this expands to start introducing an implementation of the C standard library

  compilation:
    make
*/

#define P2_TARGET_MHZ   200
#include "propeller2.h"
#include "sys/p2es_clock.h"

typedef __builtin_va_list va_list;
#define va_start(v, l) __builtin_va_start(v,l)
#define va_end(v)      __builtin_va_end(v)
#define va_arg(v, l)   __builtin_va_arg(v, l)
#define va_copy(d,s)   __builtin_va_copy(d, s)

// volatile unsigned int blink1_stack[32];
// volatile unsigned int blink2_stack[32];
// volatile unsigned int blink3_stack[32];
// volatile unsigned int blink4_stack[32];

// struct led_mb_t {
//     char pin;
//     int delay;
//     unsigned int *stack;
// };

// void blink(void *par) {
//     led_mb_t *led = (led_mb_t*)par;

//     dirh(led->pin);

//     while(1) {
//         outl(led->pin);
//         waitx(led->delay);
//         outh(led->pin);
//         waitx(led->delay);
//     }
// }

// void start_blinks(led_mb_t *led, ...) {
//     va_list args;
//     va_start(args, led);

//     led_mb_t *l = va_arg(args, led_mb_t*);
//     while(l != 0) {
//         cognew(blink, (int)l, (unsigned int*)(l->stack));
//         l = va_arg(args, led_mb_t*);
//     }

// }

// sum n numbers
int sum(int n, ...) {
    va_list args;
    va_start(args, n);

    int v = va_arg(args, int);

    // for (int i = 0; i < n-1; i++) {
    //     v += va_arg(args, int);
    // }

    return n+v;
}

int main() {
    // clkset(_SETFREQ, _CLOCKFREQ);
    dirh(56);
    outl(56);

    // led_mb_t led1 = {56, 100000000, (unsigned int*)blink1_stack};
    // led_mb_t led2 = {57, 200000000, (unsigned int*)blink2_stack};
    // led_mb_t led3 = {56, 300000000, (unsigned int*)blink3_stack};
    // led_mb_t led4 = {57, 400000000, (unsigned int*)blink4_stack};

    // start_blinks(&led1, &led2, &led3, &led4, 0);

    sum(5, 10, 20, 30, 40, 50);

    outh(56);

    while(1);
    return 0;
}
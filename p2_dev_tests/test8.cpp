/*
  This tests variadic functions. One of the last things needed before we can begin implementing a much more complex library
  Also, this expands to start introducing an implementation of the C standard library

  compilation:
    make
*/

#include "propeller2.h"

unsigned int blink1_stack[32];
unsigned int blink2_stack[32];

struct led_mb_t {
    char pin;
    int delay;
};

void blink(void *par) {
    led_mb_t *led = (led_mb_t*)par;

    dirh(led->pin);

    while(1) {
        outh(led->pin);
        waitx(led->delay);
        outl(led->pin);
        waitx(led->delay);
    }
}

void start_blinks(...) {
    va_list args;
}

int main() {

    led_mb_t led1 = {56, 25000000};
    led_mb_t led2 = {58, 5000000};

    cognew(blink, (int)&led1, (int*)blink1_stack);
    cognew(blink, (int)&led2, (int*)blink2_stack);

    while(1);

    return 0;
}
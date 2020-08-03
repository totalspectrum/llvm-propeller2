/*
  Test libcalls

  compilation:
    make
*/

#define P2_TARGET_MHZ   200
#include "propeller2.h"
#include "sys/p2es_clock.h"

#include "printf.h"

#define RX_PIN 63
#define TX_PIN 62

volatile int uart_clock_per_bits;

int main() {
    clkset(_SETFREQ, _CLOCKFREQ);
    uart_clock_per_bits = uart_init(RX_PIN, TX_PIN, 230400);

    volatile int i = 10;
    volatile int j = 5;
    printf("test %d\n", 1); // don't have a puts yet, so need to always pass a parameter so the optimizer doesn't replace
                            // this is a puts call
    printf("%d/%d = %d\n", i, j, i/j);

    i = -10;
    j = 5;
    printf("test %d\n", 2);
    printf("%d/%d = %d\n", i, j, i/j);

    i = 10;
    j = -5;
    printf("test %d\n", 3);
    printf("%d/%d = %d\n", i, j, i/j);

    i = -10;
    j = -5;
    printf("test %d\n", 4);
    printf("%d/%d = %d\n", i, j, i/j);

    i = -10;
    j = 5;
    printf("test %d\n", 5);
    printf("%d*%d = %d\n", i, j, i*j);

    while(1);
    return 0;
}
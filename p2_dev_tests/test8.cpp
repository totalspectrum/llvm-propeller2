/*
  This tests UART printing

  compilation:
    make
*/

#define P2_TARGET_MHZ   200
#include "propeller2.h"
#include "sys/p2es_clock.h"

#define RX_PIN 63
#define TX_PIN 62

unsigned int blink1_stack[32];
unsigned int blink2_stack[32];

struct led_mb_t {
    char pin;
    int delay;
};

int uart_clock_per_bits;

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

/*
 * print a 0 terminated string
 */
void uart_str(const char *str) {
    while (*str) {
        uart_putc(*str);
        str++;
        waitx(uart_clock_per_bits*10); // wait for the bits to send
    }
}

int main() {

    clkset(_SETFREQ, _CLOCKFREQ);

    uart_clock_per_bits = uart_init(RX_PIN, TX_PIN, 230400);

    led_mb_t led1 = {56, _CLOCKFREQ};
    led_mb_t led2 = {58, _CLOCKFREQ/2};

    cognew(blink, (int)&led1, blink1_stack);
    cognew(blink, (int)&led2, blink2_stack);

    uart_str("Hello World!");

    while(1);

    return 0;
}
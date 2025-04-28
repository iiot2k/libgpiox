/*
 * example blinks output, blink period is switch with input
 *
 * connect switch to gpio pin 21 and to ground 
 * connect led to gpio pin 20 with resistor (220-470 Ohm) and to ground 
 * 
 * build:
 * > make
 *
 * run:
 * > ./blink
 *
 */

#include <stdio.h>
#include <csignal>
#include <cstdlib>

#include "../include/gpiox.h"
#include "../include/c_timer.h"

#define INPUT_PIN  21
#define OUTPUT_PIN 20

#define DEBOUNCE_US 10000 // us
#define PRINT_MSG true // print error on console

#define BLINK_TIME_MS_1 500 // ms
#define BLINK_TIME_MS_2 1500 // ms

// only one chip
c_chip chip;

// gpio
c_gpio gpio1(&chip, PRINT_MSG);
c_gpio gpio2(&chip, PRINT_MSG);

// signal handler
void onCtrlC(int signum)
{
    puts("\n program stopped");

    // clear output
    gpio2.write(0);

    exit(signum);
}

int main()
{
    signal(SIGINT, onCtrlC);

    puts("*** blink C++ example ***");
    puts("stop program with Ctrl+C");

    // init input
    if (!gpio1.init(INPUT_PIN, GPIO_MODE_INPUT_PULLUP, DEBOUNCE_US))
        return 1;

    // init output
    if (!gpio2.init(OUTPUT_PIN, GPIO_MODE_OUTPUT))
        return 1;

    // create timer
    c_timer timer;

    // blink output
    while(1)
    {
        int32_t input_val = gpio1.read();

        // read input
        if (input_val == -1)
            return 1;

        // set blink rate depends on input state
        int32_t t_ms = (input_val == 1) ? BLINK_TIME_MS_1 : BLINK_TIME_MS_2;

        // sleep 1s
        timer.sleep_ms(t_ms);

        // toggle output
        gpio2.toggle();
    }

    return 0;
}


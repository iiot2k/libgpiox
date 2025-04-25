/*
 * example watch input and on change set output
 *
 * connect switch to gpio pin 21 and to ground 
 * connect led to gpio pin 20 with resistor (220-470 Ohm) and to ground 
 * 
 * build:
 * > make
 *
 * run:
 * > ./watch
 *
 */

#include <stdio.h>
#include <csignal>
#include <cstdlib>

#include "../include/gpiox.h"

#define INPUT_PIN  21
#define OUTPUT_PIN 20

#define DEBOUNCE_US 10000 // us
#define PRINT_MSG true // print error on console

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

    puts("*** watch C++ example ***");
    puts("stop program with Ctrl+C");

    // init input with edge
    if (!gpio1.init(INPUT_PIN, GPIO_MODE_INPUT_PULLUP, DEBOUNCE_US, GPIO_EDGE_BOTH))
    	return 1;

    // init output
    if (!gpio2.init(OUTPUT_PIN, GPIO_MODE_OUTPUT))
    	return 1;
    
    uint32_t edge;

    // watch input
    while(gpio1.watch(edge))
    {
        // print edge
        if (edge == GPIO_EDGE_RISING)
            printf("rising edge occurs on pin %d\n", gpio1.get_pin());
        else if (edge == GPIO_EDGE_FALLING)
            printf("falling edge occurs on pin %d\n", gpio1.get_pin());
        
        // read input and set output
        gpio2.write(gpio1.read());
    }

    return 0;
}


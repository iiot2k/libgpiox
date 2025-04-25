/*
 * example watch multiple inputs in thread
 *
 * connect switches to gpio pins 21 and 20 and common to ground 
 *
 * build:
 * > make
 *
 * run:
 * > ./watch_thread
 *
 */

#include <stdio.h>
#include <csignal>
#include <cstdlib>

#include "../include/gpiox.h"
#include "../include/c_worker.h"

#define INPUT_PIN1 21
#define INPUT_PIN2 20
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

    exit(signum);
}

// derived watch worker class
class c_watch : public c_worker
{
public:
    c_watch(c_gpio* gpio)
    {
        m_gpio = gpio;
    }

    ~c_watch() {}

    // execute thread 
    void Execute() override
    {
        uint32_t edge;

        // watch input
        while(m_gpio->watch(edge))
        {
            // print edge
            if (edge == GPIO_EDGE_RISING)
                printf("rising edge occurs on pin %d\n", m_gpio->get_pin());
            else if (edge == GPIO_EDGE_FALLING)
                printf("falling edge occurs on pin %d\n", m_gpio->get_pin());
        }
    }

private:
    c_gpio* m_gpio;
};

int main()
{
    signal(SIGINT, onCtrlC);

    puts("*** multi watch C++ example ***");
    puts("stop program with Ctrl+C");

    // init input 1 with edge
    if (!gpio1.init(INPUT_PIN1, GPIO_MODE_INPUT_PULLUP, DEBOUNCE_US, GPIO_EDGE_BOTH))
        return 1;

    // init input 2 with edge
    if (!gpio2.init(INPUT_PIN2, GPIO_MODE_INPUT_PULLUP, DEBOUNCE_US, GPIO_EDGE_FALLING))
        return 1;

    // create watch thread 1
    c_watch* wk1 = new c_watch(&gpio1);

    // start watch thread 1 and return immediately
    wk1->Queue(false);

    // create watch thread 2
    c_watch* wk2 = new c_watch(&gpio2);

    // start watch thread 2 and wait until ends (never)
    wk2->Queue(true);

    return 0;
}


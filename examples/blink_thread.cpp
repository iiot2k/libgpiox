/*
 * example blinks multiple outputs in thread
 *
 * connect leds to gpio pins 20 and 21 with resistors (220-470 Ohm) and common to ground 
 * 
 * build:
 * > make
 *
 * run:
 * > ./blink_thread
 *
 */

#include <stdio.h>
#include <csignal>
#include <cstdlib>

#include "../include/gpiox.h"
#include "../include/c_worker.h"

#define OUTPUT_PIN1 21
#define OUTPUT_PIN2 20

#define BLINK_TIME_MS_1 500 // ms
#define BLINK_TIME_MS_2 1500 // ms

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

    // clear outputs
    gpio1.write(0);
    gpio2.write(0);

    exit(signum);
}

// derived blink worker class
class c_blink : public c_worker
{
public:
    c_blink(c_gpio* gpio, int32_t period)
    {
        m_gpio = gpio;
        m_period = period;
    }

    ~c_blink() {}

    // execute thread 
    void Execute() override
    {
        // blink output
        while(1)
        {
            // delay sleep
            usleep(m_period*1000);

            // toggle output
            m_gpio->toggle();
        }
    }

private:
    int32_t m_period;
    c_gpio* m_gpio;
};

int main()
{
    signal(SIGINT, onCtrlC);

    puts("*** multi blink C++ example ***");
    puts("stop program with Ctrl+C");

    // init output 1
    if (!gpio1.init(OUTPUT_PIN1, GPIO_MODE_OUTPUT))
    	return 1;

    // init output 2
    if (!gpio2.init(OUTPUT_PIN2, GPIO_MODE_OUTPUT))
    	return 1;

    // create blink thread 1
    c_blink* wk1 = new c_blink(&gpio1, BLINK_TIME_MS_1);

    // start blink thread 1 and return immediately
    wk1->Queue(false);

    // create blink thread 2
    c_blink* wk2 = new c_blink(&gpio2, BLINK_TIME_MS_2);

    // start blink thread 2 and wait until ends (never)
    wk2->Queue(true);

    return 0;
}


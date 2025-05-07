/*
 * example implements counter with watch
 *
 * connect switches to gpio pins 21 and 16 and common to ground 
 * connect led to gpio pin 20 with resistor (220-470 Ohm) and to ground 
 *
 * build:
 * > make
 *
 * run:
 * > ./counter
 *
 */

#include <atomic>

#include <stdio.h>
#include <csignal>
#include <cstdlib>

#include "../include/gpiox.h"
#include "../include/c_worker.h"

#define COUNT_PIN 21
#define RESET_PIN 16
#define OUTPUT_PIN 20
#define DEBOUNCE_US 10000 // us

#define PRINT_MSG true // print error on console

#define COUNTER_MAX 10 // counter max

// only one chip
c_chip chip;

// gpio
c_gpio gpio_count(&chip, PRINT_MSG);
c_gpio gpio_reset(&chip, PRINT_MSG);
c_gpio gpio_output(&chip, PRINT_MSG);

// signal handler
void onCtrlC(int signum)
{
    puts("\n program stopped");
    exit(signum);
}

// derived counter worker class
class c_counter : public c_worker
{
public:
    c_counter(c_gpio* gpio_counter, c_gpio* gpio_output, std::atomic_uint* cnt)
    {
        m_gpio_counter = gpio_counter;
        m_gpio_output = gpio_output;
        m_cnt = cnt;
    }

    ~c_counter() {}

    // execute thread 
    void Execute() override
    {
        uint32_t edge;

        // watch counter input
        while(m_gpio_counter->watch(edge))
        {
            if (m_cnt->load() < COUNTER_MAX) // count if max not reached
            {
                m_cnt->store(m_cnt->load() + 1); // increment counter
                printf("count up: %d\n", m_cnt->load());
            }

            if (m_cnt->load() >= COUNTER_MAX) // count max reached
            {
                m_gpio_output->write(1); // turn output on
                printf("counter at end: %d\n", m_cnt->load());
            }
        }
    }

private:
    c_gpio* m_gpio_counter;
    c_gpio* m_gpio_output;
    std::atomic_uint* m_cnt;
};

// derived reset worker class
class c_reset : public c_worker
{
public:
    c_reset(c_gpio* gpio_reset, c_gpio* gpio_output, std::atomic_uint* cnt)
    {
        m_gpio_reset = gpio_reset;
        m_gpio_output = gpio_output;
        m_cnt = cnt;
    }

    ~c_reset() {}

    // execute thread 
    void Execute() override
    {
        uint32_t edge;

        // watch reset input
        while(m_gpio_reset->watch(edge))
        {
            m_gpio_output->write(0); // turn output off
            m_cnt->store(0); // reset counter
            printf("reset counter: %d\n", m_cnt->load());
        }
    }

private:
    c_gpio* m_gpio_reset;
    c_gpio* m_gpio_output;
    std::atomic_uint* m_cnt;
};

int main()
{
    signal(SIGINT, onCtrlC);

    puts("*** counter C++ example ***");
    puts("stop program with Ctrl+C");
  
    // init count pin with edge
    if (!gpio_count.init(COUNT_PIN, GPIO_MODE_INPUT_PULLUP, DEBOUNCE_US, GPIO_EDGE_RISING))
        return 1;

    // init reset pin with edge
    if (!gpio_reset.init(RESET_PIN, GPIO_MODE_INPUT_PULLUP, DEBOUNCE_US, GPIO_EDGE_RISING))
        return 1;

    // init output
    if (!gpio_output.init(OUTPUT_PIN, GPIO_MODE_OUTPUT))
        return 1;
    
    // the thread safe counter
    std::atomic_uint counter = 0;

    // create counter thread
    c_counter* wk1 = new c_counter(&gpio_count, &gpio_output, &counter);

    // start counter thread and return immediately
    wk1->Queue(false);

    // create reset thread
    c_reset* wk2 = new c_reset(&gpio_reset, &gpio_output, &counter);

    // start reset thread and wait until ends (never)
    wk2->Queue(true);

    return 0;
}


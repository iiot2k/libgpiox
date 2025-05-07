/*
 * gpiox library header only file
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * gpiox.h
 *
 */

#pragma once

#include <mutex>
using namespace std;

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include <poll.h>

#include "gpio.h"

// count of pins on rpi
#define N_PIN 28

/**
 * @brief gpio modes
 * all modes are active high
 */
enum {
    GPIO_MODE_INPUT = 0, // floating input
    GPIO_MODE_INPUT_PULLDOWN,   // input with pull-down resistor, 1: on connect to +3.3V
    GPIO_MODE_INPUT_PULLUP,     // input with pull-up resistor, 1: on connect to ground
    GPIO_MODE_OUTPUT,           // output, 0: connect to ground, 1: +3.3V
    GPIO_MODE_OUTPUT_SOURCE,    // output source, 0: Hi-Z, 1: +3.3V
    GPIO_MODE_OUTPUT_SINK,      // output sink,  0: Hi-Z, 1: connect to ground
};

/**
 * @brief gpio edge modes
 */
enum {
    GPIO_EDGE_RISING = 0, // rising edge
    GPIO_EDGE_FALLING,    // falling edge
    GPIO_EDGE_BOTH,       // rising + falling edge
    GPIO_EDGE_NONE,       // no edge
};

/**
 * @brief class c_chip
 * 
 * since kernel 6.6.45 all chip address is 0
 */
class c_chip
{
public:
    /**
     * @brief class constuctor
     */
    c_chip()
    {
        // open chip4
        m_fd = open("/dev/gpiochip4", O_RDWR | O_CLOEXEC);

        // open chip0 if chip4 not open
        if (m_fd == -1)
            m_fd = open("/dev/gpiochip0", O_RDWR | O_CLOEXEC);
    };

    /**
     * @brief class destructor
     */
    ~c_chip()
    {
        // close chip handle
        if (m_fd != -1)
            close(m_fd);
    };

    /**
     * @brief returns chip handle
     */
    inline int32_t get_fd() { return m_fd; }

private:
    int32_t m_fd; // file handle of chip
};

/**
 * @brief class c_gpio
 */
class c_gpio
{
public:
    /**
     * @brief class constuctor
     * @note call setchip()
     */
    c_gpio()
    {
        m_pin = -1;
        m_fd = -1;
        m_print_msg = false;
        m_chip = NULL;
    }

    /**
     * @brief class constuctor
     * @param chip pointer to chip
     * @param print_msg flag for print error messages, true = on
     */
    c_gpio(c_chip* chip, bool print_msg = false)
    {
        m_pin = -1;
        m_fd = -1;
        setchip(chip, print_msg);
    }

    /**
     * @brief set chip and message flag
     * @param chip pointer to chip
     * @param print_msg flag for print error messages, true = on
     */
    void setchip(c_chip* chip, bool print_msg = false)
    {
        m_print_msg = print_msg;
        m_chip = chip;
    }

    /**
     * @brief class destructor
     */
    ~c_gpio()
    {
        deinit();
    }

    /**
     * @brief deinits gpio pin
     */
    void deinit()
    {
        // close pin handle if open
        if (m_fd != -1)
            close(m_fd);
        m_pin = -1;
        m_fd = -1;
    }

    /**
     * @brief returns gpio pin number
     * @returns 0..27, -1 if not init
     */
    int32_t get_pin() { return m_pin; }

    /**
     * @brief clears message buffer
     */
    void clear_error()
    {
        m_msg.erase();
    }

    /**
     * @brief returns error message
     * @returns message
     */
    const char* get_error()
    {
        return m_msg.c_str();
    }

    /**
     * @brief stores message and prints error message if enabled on stderr
     * @param msg message to print, if NULL errno is print
     * @returns always false
     */
    bool print_error(const char* msg = NULL)
    {
        if (msg == NULL)
        {
            m_msg = "gpiox: "; 
            m_msg += strerror(errno);
        }
        else
            m_msg = msg;

        // if flag set print error on console
        if (m_print_msg)
            puts(get_error());

        return false;
    }

    /**
     * @brief inits gpio pin
     * @param pin gpio pin 0..27
     * @param mode gpio mode GPIO_MODE_..
     * @param setval input: debounce time in ms, output: gpio state 0/1
     * @param edge input edge GPIO_EDGE_.., ignored on output
     * @returns true: ok, false: error
     */
    bool init(uint32_t pin, uint32_t mode, uint32_t setval = 0, uint32_t edge = GPIO_EDGE_NONE)
    {
        // clear error
        clear_error();

        // check pin
        if (pin >= N_PIN)
            return print_error("gpiox: invalid pin");

        // valid chip ?
        if (m_chip == NULL)
            return print_error("gpiox: invalid chip");

        // check if chip is open
        if (m_chip->get_fd() == -1)
            return print_error("gpiox: chip not open");

        // close gpio
        deinit();

        // init line request
        gpio_v2_line_request line_request;

        memset(&line_request, 0, sizeof(line_request));

        line_request.num_lines = 1;
        line_request.offsets[0] = pin;

        // set gpio configuration
        switch(mode)
        {
        case GPIO_MODE_INPUT:
            line_request.config.flags = GPIO_V2_LINE_FLAG_INPUT + GPIO_V2_LINE_FLAG_BIAS_DISABLED;
            set_line_debounce_us(line_request.config, setval);
            set_edge_value(line_request.config, edge);
            break;

        case GPIO_MODE_INPUT_PULLDOWN:
            line_request.config.flags = GPIO_V2_LINE_FLAG_INPUT + GPIO_V2_LINE_FLAG_BIAS_PULL_DOWN;
            set_line_debounce_us(line_request.config, setval);
            set_edge_value(line_request.config, edge);
            break;

        case GPIO_MODE_INPUT_PULLUP:
            line_request.config.flags = GPIO_V2_LINE_FLAG_INPUT + GPIO_V2_LINE_FLAG_BIAS_PULL_UP + GPIO_V2_LINE_FLAG_ACTIVE_LOW;
            set_line_debounce_us(line_request.config, setval);
            set_edge_value(line_request.config, edge);
            break;

        case GPIO_MODE_OUTPUT:
            line_request.config.flags = GPIO_V2_LINE_FLAG_OUTPUT;
            set_line_value(line_request.config, setval);
            break;

        case GPIO_MODE_OUTPUT_SOURCE:
            line_request.config.flags = GPIO_V2_LINE_FLAG_OUTPUT + GPIO_V2_LINE_FLAG_OPEN_SOURCE;
            set_line_value(line_request.config, setval);
            break;

        case GPIO_MODE_OUTPUT_SINK:
            line_request.config.flags = GPIO_V2_LINE_FLAG_OUTPUT + GPIO_V2_LINE_FLAG_OPEN_DRAIN + GPIO_V2_LINE_FLAG_ACTIVE_LOW;
            set_line_value(line_request.config, setval);
            break;

        default:
            return print_error("gpiox: invalid mode");
        }

        // init gpio pin
        if (ioctl(m_chip->get_fd(), GPIO_V2_GET_LINE_IOCTL, &line_request) == -1)
            return print_error();

        // check for valid handle
        if (line_request.fd < 0)
            return print_error();

        // set file handle
        m_fd = line_request.fd;

        // set pin
        m_pin = pin;

        return true;
    }

    /**
     * @brief reads gpio
     * @param invert if true return inverted state
     * @returns ok: 0/1, error: -1
     */
    int32_t read(bool invert = false)
    {
        const lock_guard<mutex> lock(m_mtx);

        // clear error
        clear_error();

        if (m_pin == -1)
        {
            print_error("gpiox: gpio not init");
            return -1;
        }

        gpio_v2_line_values line_values;
        line_values.mask = 1;
        line_values.bits = 0;

        // read gpio pin
        if (ioctl(m_fd, GPIO_V2_LINE_GET_VALUES_IOCTL, &line_values))
        {
            print_error();
            return -1;
        }

        if (invert)
            return (line_values.bits == 1) ? 0 : 1;
        else
            return (line_values.bits == 1) ? 1 : 0;
    }

    /**
     * @brief sets pin state
     * @param val state to write 0 or 1
     * @param invert if true write inverted state
     * @returns false: error, true: ok
     */
    bool write(int32_t val, bool invert = false)
    {
        const lock_guard<mutex> lock(m_mtx);

        // clear error
        clear_error();

        if (m_pin == -1)
            return print_error("gpiox: gpio not init");

        gpio_v2_line_values line_values;
        line_values.mask = 1;

        if (invert)
            line_values.bits = val > 0 ? 0 : 1;
        else
            line_values.bits = val > 0 ? 1 : 0;

        // write gpio pin
        if (ioctl(m_fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &line_values) == -1)
            return print_error();

        return true;
    }

    /**
     * @brief toggle output
     * @param invert if true write inverted state
     * @returns false: error, true: ok
     */
    bool toggle(bool invert = false)
    {
        // read gpio
        int32_t value = read(invert);

        if (value == -1)
            return false;

        // toggle value
        value = (value == 1) ? 0 : 1;

        // write gpio    
        return write(value);
    }

    /**
     * @brief watch gpio for changes
     * @param edge receives edge constant GPIO_EDGE_RISING or GPIO_EDGE_FALLING
     * @returns true: valid gpio event, false: error
     */
    bool watch(uint32_t& edge)
    {
        // clear error
        clear_error();

        if (m_pin == -1)
            return print_error("gpiox: not init");

        // event data
        gpio_v2_line_event event_data;

        // poll data
        pollfd pfd = { .fd = m_fd, .events = POLLIN, .revents = 0 };

        while(1)
        {
            // wait for event
            if (poll(&pfd, 1, -1) <= 0)
                return print_error();

            // read event data
            int32_t ret = ::read(m_fd, &event_data, sizeof(event_data));

            // check return code
            if (ret == -1)
            {
                // read again
                if (errno == -EAGAIN)
                    continue;
                else
                    return print_error();
            }

            // check if read all data
            if (ret != sizeof(event_data))
                return print_error();

            break;
        }

        // set event edge
        switch(event_data.id)
        {
        case GPIO_V2_LINE_EVENT_RISING_EDGE:
            edge = GPIO_EDGE_RISING;
            break;
        case GPIO_V2_LINE_EVENT_FALLING_EDGE:
            edge = GPIO_EDGE_FALLING;
            break;
        default:
            edge = GPIO_EDGE_NONE;
        }

        return true;
    }

private:
    // set debounce parameter
    void set_line_debounce_us(gpio_v2_line_config& line_config, uint32_t debounce)
    {
        line_config.num_attrs = 1;
        line_config.attrs[0].mask = 1;
        line_config.attrs[0].attr.id = GPIO_V2_LINE_ATTR_ID_DEBOUNCE;
        line_config.attrs[0].attr.debounce_period_us = debounce;
    }

    // set output state parameter
    void set_line_value(gpio_v2_line_config& line_config, uint32_t setval)
    {
        line_config.num_attrs = 1;
        line_config.attrs[0].mask = 1;
        line_config.attrs[0].attr.id = GPIO_V2_LINE_ATTR_ID_OUTPUT_VALUES;
        line_config.attrs[0].attr.values = setval > 0 ? 1 : 0;
    }

    // set edge parameter
    void set_edge_value(gpio_v2_line_config& line_config, uint32_t edge)
    {
        // set edge parameter
        switch(edge)
        {
        case GPIO_EDGE_RISING:
            line_config.flags += GPIO_V2_LINE_FLAG_EDGE_RISING;
            break;
        case GPIO_EDGE_FALLING:
            line_config.flags += GPIO_V2_LINE_FLAG_EDGE_FALLING;
            break;
        case GPIO_EDGE_BOTH:
            line_config.flags += GPIO_V2_LINE_FLAG_EDGE_RISING + GPIO_V2_LINE_FLAG_EDGE_FALLING;
            break;
        default:
        case GPIO_EDGE_NONE:
            break;
        }
    }

    c_chip* m_chip;   // chip
    int32_t m_pin;    // gpio pin
    int32_t m_fd;     // gpio pin handle
    bool m_print_msg; // flag for print message
    string m_msg;     // store for messages
    mutex m_mtx;      // lock mutex
};

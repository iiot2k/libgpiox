/*
 * hx711 driver
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * c_hx711.h
 *
 */

#pragma once

#include <mutex>
#include <cmath>
using namespace std;

#include "../../include/gpiox.h"
#include "../../include/c_timer.h"
#include "../../include/c_priority.h"

/**
 * @brief gain and channel
 */
enum {
    GAIN_A128 = 1, // channel A gain 128
    GAIN_B32 = 2,  // channel B gain 32
    GAIN_A64 = 3,  // channel A gain 64
};

/**
 * @brief class c_hx711
 */
class c_hx711
{
public:
    /**
     * @brief class constuctor
     */
    c_hx711()
    {
        m_print_msg = false;
        m_gain = GAIN_A128;
    }

    /**
     * @brief class destructor
     */
    ~c_hx711()
    {
        // power down hx711
        power_down();
    }

    /**
     * @brief power down hx711
     */
    void power_down()
    {
        m_gpio_cl.write(0);
        m_gpio_cl.write(1);
    }

    /**
     * @brief inits dt + cl pins and hx711 
     * @param chip pointer to chip
     * @param pin_dt dt pin (0..27)
     * @param pin_cl cl pin (0..27)
     * @param print_msg flag for print error messages, true = on
     * @returns true: ok, false: error
     */
    bool init(c_chip* chip, uint32_t pin_dt, uint32_t pin_cl, bool print_msg = false)
    {
        m_gpio_dt.setchip(chip, print_msg); // dt pin
        m_gpio_cl.setchip(chip, print_msg); // cl pin
        m_print_msg = print_msg;

        // init cl pin
        if (!m_gpio_cl.init(pin_cl, GPIO_MODE_OUTPUT))
            return false;

        // init dt pin
        if (!m_gpio_dt.init(pin_dt, GPIO_MODE_INPUT))
            return false;

        // wait hx711 ready
        return wait_ready();
    }

    /**
     * @brief read hx711
     * @param value receives hx711 adc value
     * @param gain GAIN_..
     * @param nread count of reads for average (1..)
     * @returns true: ok, false: error
     */
    bool read(double& value, uint32_t gain = GAIN_A128, uint32_t nread = 10)
    {
        const lock_guard<mutex> lock(m_mtx);

        // check gain
        if ((gain < GAIN_A128) || (gain > GAIN_A64))
            return print_err("invalid gain");

        // adjust nread
        if (nread == 0)
            nread = 10;

        value = 0.0;

        // if gain changed set gain and port with one call
        if (gain != m_gain)
        {
            if (!wait_ready())
                return false;
    
            read_raw(gain);
            m_gain = gain;
        }
    
        // read multiple times
        for (uint32_t i=0; i < nread; i++)
        {
            if (!wait_ready())
                return false;
    
            value += double(read_raw(gain));
        }
    
        // build average
        value = abs(round(value / double(nread)));
    
        return true;
    }

private:
    /**
     * @brief waits hx711 ready until timeout
     * @returns true: ok, false: timeout
     */
    bool wait_ready()
    {
        // wait 600ms for ready
        for(uint32_t t=0; t < 60; t++)
        {
            if (m_gpio_dt.read() == 0)
                return true;
            
            m_timer.sleep_ms(10); // delay 10ms
        }

        return print_err("wait timeout");
    }

    /**
     * @brief read hx711 raw data
     * @param gain GAIN_..
     * @returns hx711 adc data
     */
    int32_t read_raw(uint32_t gain)
    {
        // switch priority
        c_priority priority;

        uint32_t data = 0;
    
        // read from hx711
        for (int32_t i = 0; i < 24; i++)
        {
            m_gpio_cl.write(1);
            m_timer.delay_ns(100l); // delay 100ns
            data = (data << 1) | m_gpio_dt.read();
            m_gpio_cl.write(0);
            m_timer.delay_us(1l); // delay 1us
        }
    
        // set gain
        for (uint8_t i = 0; i < gain; i++)
        {
            m_gpio_cl.write(1);
            m_timer.delay_us(1l); // delay 1us
            m_gpio_cl.write(0);
        }
    
        // convert to int32_t
        if (data & 0x800000)
            data |= 0xFF000000;

        return int32_t(data);
    }

    /**
     * @brief prints error message if enabled on stderr
     * @param msg message to print
     * @returns always false
     */
    bool print_err(const char* msg)
    {
        if (m_print_msg)
            printf("err: %s\n", msg);

        return false;
    }

    c_gpio m_gpio_dt; // hx711 data gpio
    c_gpio m_gpio_cl; // hx711 clock gpio

    uint32_t m_gain;  // save gain
    bool m_print_msg; // flag for print message
    mutex m_mtx;      // lock mutex
    c_timer m_timer;  // timer
};

/*
 * ds18b20 driver
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * c_ds18b20.h
 *
 */

#pragma once

#include <mutex>
using namespace std;

#include "../../include/gpiox.h"
#include "../../include/c_timer.h"
#include "../../include/c_priority.h"

// sensor resolution
enum {
    RES_SENSOR_9 = 0,
    RES_SENSOR_10,
    RES_SENSOR_11,
    RES_SENSOR_12,
};

// sensor id (64bit)
typedef unsigned long long sensor_id;

// One wire command codes
#define SKIP_ROM   0xCC
#define MATCH_ROM  0x55
#define SEARCH_ROM 0xF0
#define COVERT_T   0x44
#define WRITE_PAD  0x4E
#define COPY_PAD   0x48
#define READ_PAD   0xBE

class c_ds18b20
{
public:
    /**
     * @brief class constuctor
     */
    c_ds18b20()
    {
        m_print_msg = false;
        m_res = RES_SENSOR_12;
    }

    /**
     * @brief inits sensor gpio pin
     * @param chip pointer to chip
     * @param pin sensor pin (0..27)
     * @param print_msg flag for print error messages, true = on
     * @returns true: ok, false: error
     */
    bool init(c_chip* chip, uint32_t pin, bool print_msg = false)
    {
        m_gpio.setchip(chip, print_msg);
        m_print_msg = print_msg;
        
        // init gpio pin
        return m_gpio.init(pin, GPIO_MODE_OUTPUT);
    }

    /**
     * @brief set resolution on all sensors on bus
     * @param red sensor resolution RES_SENSOR_..
     * @returns false if no sensor on bus or error
     */
    bool set_resolution(uint32_t res)
    {
        c_priority priority;
    
        // resets 1-wire bus
        if (!reset())
            return false;
    
        uint8_t cfg;

        // set parameter depends on on sensor resolution
        switch(res)
        {
        case RES_SENSOR_9:
            cfg = 0x1f;
            break;
        case RES_SENSOR_10:
            cfg = 0x3f;
            break;
        case RES_SENSOR_11:
            cfg = 0x5f;
            break;
        default:
        case RES_SENSOR_12:
            cfg = 0x7f;
            break;
        }
    
        // 1-wire set procedure for set pad
        write_byte(SKIP_ROM);
        write_byte(WRITE_PAD);
        write_byte(0xFF); // TH
        write_byte(0xFF); // TL
        write_byte(cfg); // resolution
    
        m_timer.sleep_ms(1);
        m_res = res;
    
        return true;
    }

    /**
     * @brief read one sensors with given id
     * @param s_id string id of sensor in format 28-HHHHHHHHHHHH (hex)
     * @param fh fh true for fahrenheit, false for celsius
     * @param temp fh temp receives temperature from sensor
     * @param repeat number of reads attemps 1..
     * @returns false if no sensor on bus or error
     */
    bool read_sensor(const char* s_id, bool fh, double& temp, uint8_t repeat = 10)
    {
        const lock_guard<mutex> lock(m_mtx);

        uint32_t family;
        sensor_id id;
    
        // scan string id and split to id and family
        if (sscanf(s_id, "%02X-%llX", &family, &id) != 2)
            return print_err("invalid sensor id");
    
        // build 64 bit id
        id <<= 8;
        id &= ~(0xFFULL);
        id |= (sensor_id) family;
        id |= ((sensor_id) crc8((uint8_t*) &id, 7)) << 56;

        // send start sensor conversion
        if (!start_convert())
            return false;
    
        // waits depends on sensor resolution
        switch(m_res)
        {
        case RES_SENSOR_9: // 100ms
            m_timer.sleep_ms(100l);
            break;
        case RES_SENSOR_10: // 200ms
            m_timer.sleep_ms(200l);
            break;
        case RES_SENSOR_11: // 400ms
            m_timer.sleep_ms(400l);
            break;
        default:
        case RES_SENSOR_12: // 800ms
            m_timer.sleep_ms(800l);
            break;
        }
    
        // read sensor data
        if (!read_sensor(&id, temp, repeat))
            return false;
    
        // convert to fahrenheit
        if (fh)
            temp = temp * (9.0/5.0) + 32.0;
    
        return true;
    }
    
private:
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

    /**
     * @brief crc calculation
     * @param data points to data for calculation
     * @param len length of data for calculation
     * @returns crc value
     */
    uint8_t crc8(uint8_t *data, uint8_t len)
    {
        uint8_t crc = 0;
        
        while (len--) {
            uint8_t inbyte = *data++;
            for (uint8_t i = 8; i; i--) {
                uint8_t mix = (crc ^ inbyte) & 0x01;
                crc >>= 1;
                if (mix) crc ^= 0x8C;
                inbyte >>= 1;
            }
        }

        return crc;
    }

    /**
     * @brief resets 1-wire bus
     * @returns false if no sensor on bus or error
     */
    bool reset()
    {
        int8_t bit = 1;

        // check for gpio error
        if (!set_high())
            return false;

        // 1-wire bus reset procedure
        m_timer.delay_us(10l);
        set_low();
        m_timer.delay_us(500l);
        set_high();
        m_timer.delay_us(60l);
        bit = read();
        m_timer.sleep_ms(1l);

        // 1-wire bus must set to 0 on present sensors
        if (bit != 0)
            return print_err("no sensor");

        return true;
    }

    /**
     * @brief starts conversion of sensors
     * @returns false if no sensor on bus or error
     */
    bool start_convert()
    {
        c_priority priority;

        // resets 1-wire bus
        if (!reset())
            return false;

        // send start conversion comand
        write_byte(SKIP_ROM);
        write_byte(COVERT_T);

        return true;
    }

    /**
     * @brief read sensor temperature with given 64bit id
     * @param id 64 bit sensor id 
     * @param temp temperature to receive
     * @param repeat number of reads attemps
     * @returns true: valid read, false: no sensor read or error
     */
    bool read_sensor(const sensor_id* id, double& temp, uint8_t repeat)
    {
        c_priority priority;
    
        uint8_t pad[9];
    
        // conversion of pad to short
        union {
            short SHORT;
            unsigned char CHAR[2];
        } u_temp;
   
        // read repeated
        for (uint8_t i=0; i < repeat; i++)
        {
            // reset 1-wire bus
            if (!reset())
                return false;
    
            // send 1-wire match rom command
            write_byte(MATCH_ROM);
            write_block((const uint8_t*) id, 8);

            // read sensor pad
            memset(&pad, 0, sizeof(pad));
            write_byte(READ_PAD);
            read_block((uint8_t*) &pad, 9);
    
            // check crc, on mismatch continue read
            if (crc8((uint8_t*) &pad, 8) != pad[8])
                continue;
    
            // set convert parameter
            u_temp.CHAR[0] = pad[0];
            u_temp.CHAR[1] = pad[1];
    
            // convert to °C
            temp =  0.0625 * (double)u_temp.SHORT;
    
            return true;
        }
    
        return print_err("no sensor read");
    }

    /**
     * @brief read on byte from 1-wire bus
     * @returns byte value
     */
    uint8_t read_byte()
    {
        uint8_t byte = 0;
    
        for (uint8_t i=0; i<8; i++)
        {
            set_low();
            m_timer.delay_us(1l);
            set_high();
            m_timer.delay_us(2l);
    
            if (read())
                byte |= (1 << i);
    
            m_timer.delay_us(60l);
        }
    
        return byte;
    }
    
    /**
     * @brief read multiple bytes from 1-wire bus
     * @param data points to data to receive
     * @param len length of data
     */
    void read_block(uint8_t* data, uint8_t len)
    {
        for (uint8_t i=0; i<len; i++)
            data[i] = read_byte();
    }
    
    /**
     * @brief write one byte to 1-wire bus
     * @param byte data to write
     */
    void write_byte(uint8_t byte)
    {
        for (uint8_t i=0; i<8; i++)
        {
            set_low();
                
            if (byte & (1 << i)) // write high bit
            {
                m_timer.delay_us(1l);
                set_high();
                m_timer.delay_us(60l);
            }
            else // write low bit
            {
                m_timer.delay_us(60l);
                set_high();
                m_timer.delay_us(1l);
            }
    
            m_timer.delay_us(60l);
        }
    
        m_timer.delay_us(100l);
    }
    
    /**
     * @brief write multiple bytes to 1-wire bus
     * @param data points to data to send
     * @param len length of data
     */
    void write_block(const uint8_t* data, uint8_t len)
    {
        for (uint8_t i=0; i<len; i++)
            write_byte(data[i]);
    }

    // gpio functions wrapper
    inline bool set_low()  { return m_gpio.write(0); }
    inline bool set_high() { return m_gpio.write(1); }
    inline uint8_t read()  { return m_gpio.read(); }

    c_gpio m_gpio;    // sensor gpio
    c_timer m_timer;  // used for delay and sleep
    bool m_print_msg; // flag for print message
    uint32_t m_res;   // saved resolution
    mutex m_mtx;      // lock mutex
};

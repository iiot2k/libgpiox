/*
 * example reads one ds18b20 sensor
 *
 * connect ds18b20 to gpio pin 21
 * connect pullup resistor between pin 21 and +3.3v    
 * 
 * build:
 * > make
 *
 * run:
 * > ./read one
 *
 */

#include <stdio.h>
#include <csignal>
#include <cstdlib>

#include "c_ds18b20.h"

#define SENSOR_ID  "28-HHHHHHHHHHHH" // enter your sensor id

#define SENSOR_PIN 21 // gpio pin for ds18b20
#define SENSOR_RES RES_SENSOR_9 // sensor resolution
#define FAHRENHEIT false // temp. in °C

#define PRINT_MSG true // print error on console

// only one chip
c_chip chip;

// ds18b20 driver
c_ds18b20 ds18b20;

// signal handler
void onCtrlC(int signum)
{
    puts("\n program stopped");
    exit(signum);
}

int main()
{
    signal(SIGINT, onCtrlC);

    puts("*** ds18b20 read C++ example ***");

    // create timer
    c_timer timer;

    // init ds18b20
    if (!ds18b20.init(&chip, SENSOR_PIN, PRINT_MSG))
        return 1;
    
    // set sensor resolution
    if (!ds18b20.set_resolution(SENSOR_RES);
        return 1;

    // read loop
    while(1)
    {
        double temp;
        
        // read ds18b20 sensor
        if (!ds18b20.read_sensor(SENSOR_ID, FAHRENHEIT, temp))
            return 1;

        printf("temperature: %.1f°C\n", temp);
        
        // sleep 3s
        timer.sleep_s(3);
    }

    return 0;
}


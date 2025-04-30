/*
 * example scans for ds18b20 sensors and read temperatures
 *
 * connect ds18b20 data pin to gpio pin 21
 * connect pullup resistor between pin 21 and +3.3v    
 * 
 * build:
 * > make
 *
 * run:
 * > ./scan_read
 *
 */

#include <stdio.h>

#include "c_ds18b20.h"

#define SENSOR_PIN 21 // gpio pin for ds18b20
#define SENSOR_RES RES_SENSOR_9 // sensor resolution
#define FAHRENHEIT false // temp. in °C
#define PRINT_MSG true // print error on console

// only one chip
c_chip chip;

// ds18b20 driver
c_ds18b20 ds18b20;

// idlist
vector<sensor_id> idlist;

// list of temperature
vector<double> templist;

int main()
{
    puts("*** ds18b20 scan+read C++ example ***");

    // create timer
    c_timer timer;

    // init ds18b20
    if (!ds18b20.init(&chip, SENSOR_PIN, PRINT_MSG))
        return 1;
    
    // scan for sensors on 1-wire bus
    if (!ds18b20.scan_sensor(idlist))
        return 1;

    // check size
    if (idlist.size() == 0)
    {
        puts("no sensor on bus");
        return 1;
    }

    // set sensor resolution
    if (!ds18b20.set_resolution(SENSOR_RES))
        return 1;

    // read ds18b20 sensors from id list
    if (!ds18b20.read_sensors(idlist, FAHRENHEIT, templist))
        return 1;

    // print all temperatures
    for (double temp: templist)
        if (temp == INV_TEMP)
            puts("not read");
        else
            printf("temperature: %.1f°C\n", temp);

    return 0;
}


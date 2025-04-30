/*
 * example scans for ds18b20 sensors
 *
 * connect ds18b20 data pin to gpio pin 21
 * connect pullup resistor between pin 21 and +3.3v    
 * 
 * build:
 * > make
 *
 * run:
 * > ./scan
 *
 */

#include <stdio.h>

#include "c_ds18b20.h"

#define SENSOR_PIN 21 // gpio pin for ds18b20

#define PRINT_MSG true // print error on console

// only one chip
c_chip chip;

// ds18b20 driver
c_ds18b20 ds18b20;

// idlist
vector<sensor_id> idlist; 

int main()
{
    puts("*** ds18b20 scan C++ example ***");

    // create timer
    c_timer timer;

    // init ds18b20
    if (!ds18b20.init(&chip, SENSOR_PIN, PRINT_MSG))
        return 1;
    
    // scan for sensors on 1-wire bus
    if (!ds18b20.scan_sensor(idlist))
        return 1;
    
    // storage for id string
    string s_id;

    // print id of all sensors
    for (sensor_id id: idlist)
        if (ds18b20.idtostr(id, s_id))
            printf("%s\n", s_id.c_str());

    if (idlist.size() == 0)
        puts("no sensor on bus");

    return 0;
}


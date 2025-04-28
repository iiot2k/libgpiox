/*
 * example reads hx711 load cell
 * first run calib program to get calibration values 
 *
 * connect hx711 DOUT to gpio pin 21 and PD_SCK to gpio pin 20  
 * 
 * build:
 * > make
 *
 * run:
 * > ./measure
 *
 */
#include<iostream>
using namespace std;

#include <stdio.h>
#include <csignal>
#include <cstdlib>

#include "c_hx711.h"

#define DT_PIN 21 // gpio pin for hx711 DOUT pin
#define CL_PIN 20 // gpio pin for hx711 PD_SCK pin

#define PRINT_MSG true // print error on console
#define WUNITS "g"     // units of reference weight

// only one chip
c_chip chip;

// hx711 driver
c_hx711 hx711;

// signal handler
void onCtrlC(int signum)
{
    puts("\n program stopped");
    exit(signum);
}

int main()
{
    signal(SIGINT, onCtrlC);

    puts("*** hx711 measure C++ example ***");

    double tare_val = 0.0;
    double factor_val = 0.0;

    cout << "enter tare value: ";
    cin >> tare_val;

    cout << "enter factor value: ";
    cin >> factor_val;

    // check values
    if ((tare_val == 0.0) || (factor_val == 0.0))
    {
        puts("*** please enter tare and/or factor value ***");
        puts("*** run calib programm ***");
        return 1;
    }

    // create timer
    c_timer timer;

    // init hx711
    if (!hx711.init(&chip, DT_PIN, CL_PIN, PRINT_MSG))
        return 1;

    // read loop
    while(1)
    {
        double adc_val;

        // read hx711
        if (!hx711.read(adc_val))
            return 1;
        
        // calculate weight
        double diff = adc_val - tare_val;
        double weight = diff / factor_val;

        printf("weight: %.2f %s\n", WUNITS, weight);
        
        // sleep 1s
        timer.sleep_s(1);
    }

    return 0;
}


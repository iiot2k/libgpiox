/*
 * example reads calibration of hx711
 *
 * connect hx711 dt to gpio pin 21 and cl to gpio pin 20  
 * 
 * build:
 * > make
 *
 * run:
 * > ./calib
 *
 */

#include<iostream>
using namespace std;

#include<stdio.h>

#include "c_hx711.h"

#define DT_PIN 21 // gpio pin for hx711 DOUT pin
#define CL_PIN 20 // gpio pin for hx711 PD_SCK pin

#define PRINT_MSG true // print error on console

// only one chip
c_chip chip;

// hx711 driver
c_hx711 hx711;

int main()
{
    puts("*** hx711 calibration C++ example ***");

    // init hx711
    if (!hx711.init(&chip, DT_PIN, CL_PIN, PRINT_MSG))
        return 1;

    puts("remove any weight from load cell");
    
    double tare_value;
    
    // read tare value
    if (!hx711.read(tare_value))
        return 1;

    printf("tare-value: %.0f\n", tare_value);
    puts("!!! note tare-value !!!");
    puts("");
    puts("put known weight to load cell");

    double ref_value;

    while(1)
    {
        ref_value = 0.0;
        cout << "enter weight: ";
        cin >> ref_value;
        if (ref_value != 0.0)
            break;
    }
        
    double calib_value;
    
    // read calibration value
    if (!hx711.read(calib_value))
        return 1;

    // calculate factor
    double diff = calib_value - tare_value;
    double factor = diff / ref_value;

    printf("factor-value: %.3f\n", factor);
    puts("!!! note factor-value !!!");

    return 0;
}


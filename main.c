/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#if defined(__XC)
#include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
#include <htc.h>        /* HiTech General Include File */
#endif

#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */

#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp */





/******************************************************************************/
/* User Global Variable Declaration                                           */
/******************************************************************************/

/* i.e. uint8_t <variable_name>; */

/******************************************************************************/
/* Main Program                                                               */

/******************************************************************************/

#define DISTANCE_LIMIT_LOW      2
#define DISTANCE_LIMIT_HIGH     400
#define DISTANCE_SET            15

#define MAX_COUNT_TRY_PRESENT   8
#define MAX_COUNT_TRY_EMPTY     16
#define MAX_COUNT_TRY_DOOR      4


void main(void) {
    /* Configure the oscillator for the device */
    ConfigureOscillator();

    /* Initialize I/O and Peripherals for application */
    InitApp();

    countActionEmpty = 0;
    countActionPresent = 0;
    countActionDoor = 0;

    ULTRASONIC_TRIGGER = 0; // START POS TRIGGER LOW
    RELAY = 1;              // RELAY ON;
    LATGPIO_FLUSH;

    while (1) {
        CLRWDT();
        //check door sensor
        if (DOOR_SENSOR) {
            countActionDoor++;
        } else {
            countActionDoor = 0;
        }

        if (countActionDoor >= MAX_COUNT_TRY_DOOR) {
            //door opened will skip measure distance
            countActionPresent = MAX_COUNT_TRY_PRESENT;
            countActionEmpty = 0;
            countActionDoor = MAX_COUNT_TRY_DOOR;
            a=0;
        } else {
            // start measure disance 
            ULTRASONIC_TRIGGER = 1; //TRIGGER HIGH
            LATGPIO_FLUSH;
            __delay_us(10); //10uS Delay 
            ULTRASONIC_TRIGGER = 0; //TRIGGER LOW
            LATGPIO_FLUSH;

            __delay_ms(100);   // WAIT ECHO
            a = a + 1; //Distance Calibration
            // end measuring disance 
        }
        
        if (a >= DISTANCE_LIMIT_LOW && a <= DISTANCE_LIMIT_HIGH) //Check whether the result is valid or not
        {
            if (a <= DISTANCE_SET) {
                countActionPresent++;
                countActionEmpty = 0;                
            } else {
                countActionEmpty++;
                countActionPresent = 0;                
            }
        }

        if (countActionPresent >= MAX_COUNT_TRY_PRESENT) {
            RELAY = 1; //RELAY ON 
            LATGPIO_FLUSH;
            countActionPresent = 0;
        }
        if (countActionEmpty >= MAX_COUNT_TRY_EMPTY) {
            RELAY = 0; //RELAY OFF
            LATGPIO_FLUSH;
            countActionEmpty = 0;
        }


        __delay_ms(125);
    }
}

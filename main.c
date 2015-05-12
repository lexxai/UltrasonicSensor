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

#define MAX_COUNT_TRY_PRESENT   4
#define MAX_COUNT_TRY_EMPTY     8
#define MAX_COUNT_TRY_DOOR      6

void main(void) {
    /* Configure the oscillator for the device */
    ConfigureOscillator();

    /* Initialize I/O and Peripherals for application */
    InitApp();

    countActionEmpty = 0;
    countActionPresent = 0;
    
    //RELAY = 0; // RELAY OFF;

    while (1) {
        //CLRWDT();
        //check door sensor
//        if (DOOR_SENSOR) {
//            countActionDoor++;
//        } else {
//            countActionDoor = 0;
//        }
            
//        if (countActionDoor > MAX_COUNT_TRY_DOOR ) {
//            //door opened will skip measure distance
//            countActionDoor = 0;
//            countActionPresent = MAX_COUNT_TRY_PRESENT+1;
//        } else {
            // start measure disance 
            TMR1H = 0; //Sets the Initial Value of Timer
            TMR1L = 0; //Sets the Initial Value of Timer

            ULTRASONIC_TRIGGER = 1; //TRIGGER HIGH
            LATPORTC_FLUSH;
            __delay_us(10); //10uS Delay 
            ULTRASONIC_TRIGGER = 0; //TRIGGER LOW
            LATPORTC_FLUSH;

            while (!ULTRASONIC_ECHO); //Waiting for Echo
            TMR1ON = 1; //Timer Starts
            while (ULTRASONIC_ECHO); //Waiting for Echo goes LOW
            //__delay_us(1588);
            TMR1ON = 0; //Timer Stops

            a = (TMR1L | (TMR1H << 8)); //Reads Timer Value
            a = (int) ( a / 58); //Converts Time to Distance
            a = a + 1; //Distance Calibration
            // end measuring disance 
//        }
        
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

        if (countActionPresent > MAX_COUNT_TRY_PRESENT) {
            RELAY = 1; //RELAY ON 
            LATPORTC_FLUSH;
            countActionPresent = 0;
        }
        if (countActionEmpty > 6) {
            RELAY = 0; //RELAY OFF
            LATPORTC_FLUSH;
            countActionEmpty = 0;
        }


        __delay_ms(250);
    }
}

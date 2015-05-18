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

volatile int distance;
unsigned char countActionPresent;
unsigned char countActionEmpty;
unsigned char countActionDoor;
unsigned int TimerStateOn;

/******************************************************************************/
/* Main Program                                                               */

/******************************************************************************/

#define DISTANCE_LIMIT_LOW      2                    //cm
#define DISTANCE_LIMIT_HIGH     400                  //cm
#define DISTANCE_SET            15                   //cm

#define ECHO_WAIT               125                  //ms
#define ECHO_WAIT_PER_SEC       1000/ECHO_WAIT       //loops per second

#define MAX_COUNT_TRY_PRESENT   ECHO_WAIT_PER_SEC*1  //seconds
#define MAX_COUNT_TRY_EMPTY     ECHO_WAIT_PER_SEC*3  //seconds
#define MAX_COUNT_TRY_DOOR      ECHO_WAIT_PER_SEC*1  //seconds

#define MINUTES                 60                    //seconds
#define MAX_DOOR_TIME_ON        ECHO_WAIT_PER_SEC*MINUTES*15 //minutes
#define MAX_TIME_ON             ECHO_WAIT_PER_SEC*MINUTES*60 //minutes



void main(void) {
    /* Configure the oscillator for the device */
    ConfigureOscillator();

    /* Initialize I/O and Peripherals for application */
    InitApp();

    countActionEmpty = 0;
    countActionPresent = 0;
    countActionDoor = 0;

    ULTRASONIC_TRIGGER = 0; // START POS TRIGGER LOW
    RELAY = 1;              // RELAY ON WHEH POWER ON;
    LATGPIO_FLUSH;

    while (1) {
        CLRWDT();
        //check door sensor
        if (DOOR_SENSOR) {
            countActionDoor++;
        } else {
            countActionDoor = 0;
        }

        distance=0;                    //reset distance
        if (countActionDoor >= MAX_COUNT_TRY_DOOR) {
            //door opened will skip measure distance
            countActionPresent = MAX_COUNT_TRY_PRESENT;
            countActionEmpty = 0;
            countActionDoor = MAX_COUNT_TRY_DOOR;
        } else {
            // start measure disance 
            ULTRASONIC_TRIGGER = 1;         //TRIGGER HIGH
            LATGPIO_FLUSH;
            __delay_us(10);                 //10uS Delay 
            ULTRASONIC_TRIGGER = 0;         //TRIGGER LOW
            LATGPIO_FLUSH;
            __delay_ms(ECHO_WAIT);          // WAIT ECHO
            // end measuring disance 
        }
        
        if (distance >= DISTANCE_LIMIT_LOW && distance <= DISTANCE_LIMIT_HIGH) //Check whether the result is valid or not
        {
            if (distance <= DISTANCE_SET) {
                countActionPresent++;
                countActionEmpty = 0;                
            } else {
                countActionEmpty++;
                countActionPresent = 0;                
            }
        }

      
        if (countActionPresent >= MAX_COUNT_TRY_PRESENT) {
            RELAY = 1; //RELAY ON 
            countActionPresent = 0;
        }
        if (countActionEmpty >= MAX_COUNT_TRY_EMPTY) {
            RELAY = 0; //RELAY OFF
            countActionEmpty = 0;
            TimerStateOn = 0;
        }

        //checking safety MAX time of State ON
        if ((TimerStateOn >= MAX_TIME_ON)) {
            RELAY = 0; //RELAY OFF
            TimerStateOn = MAX_TIME_ON;
        } else if ((TimerStateOn >= MAX_DOOR_TIME_ON) && (countActionDoor >= MAX_COUNT_TRY_DOOR)) {
            RELAY = 0; //RELAY OFF
            TimerStateOn = MAX_DOOR_TIME_ON;
        } else if (RELAY) {
            TimerStateOn++;
        } else {
            TimerStateOn = 0;
        }
        
        LATGPIO_FLUSH;            
    }
}

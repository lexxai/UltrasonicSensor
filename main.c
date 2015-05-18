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

volatile uint16_t distance;
uint8_t countActionPresent;
uint8_t countActionEmpty;
uint8_t countActionDoor;
uint16_t TimerStateOn;
uint16_t TimerStateOff;
//bool    doorStateChanged = false;
bool   UltraSonicPower=true;

/******************************************************************************/
/* Main Program                                                               */

/******************************************************************************/

#define DISTANCE_LIMIT_LOW      2                    //cm
#define DISTANCE_LIMIT_HIGH     400                  //cm
#define DISTANCE_SET            15                   //cm

#define ECHO_WAIT               125                  //ms
#define ECHO_WAIT_PER_SEC       1000/ECHO_WAIT       //loops per second

#define MAX_COUNT_TRY_PRESENT   ECHO_WAIT_PER_SEC*1  //seconds
#define MAX_COUNT_TRY_EMPTY     ECHO_WAIT_PER_SEC*5  //seconds
#define MAX_COUNT_TRY_DOOR      ECHO_WAIT_PER_SEC*1  //seconds

#define MINUTES                 60                    //seconds
#define MAX_DOOR_TIME_ON        ECHO_WAIT_PER_SEC*MINUTES*15 //minutes
#define MAX_TIME_ON             ECHO_WAIT_PER_SEC*MINUTES*60 //minutes
#define TIME_OFF_DELAY          ECHO_WAIT_PER_SEC*MINUTES/2 //minutes



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
        
        if (UltraSonicPower) {
            // power on of ultrasonic module
            ULTRASONIC_POWER = true;
            LATGPIO_FLUSH;
            __delay_us(10); //10uS Delay for start module
        }
        
        //check door sensor
        if (DOOR_SENSOR) {
            countActionDoor++;
        } else {
            countActionDoor = 0;
        }

        distance=0;                    //reset distance
       
        // checking door sensor
        if (countActionDoor >= MAX_COUNT_TRY_DOOR) {
            //if door opened, will skip measure distance
            countActionPresent = MAX_COUNT_TRY_PRESENT;
            countActionEmpty = 0;
            countActionDoor = MAX_COUNT_TRY_DOOR;
            __delay_ms(ECHO_WAIT);          // SIMULTATE WAIT ECHO
            UltraSonicPower=true;
        } else {
            // if door opened 
            // start measure disance 
            ULTRASONIC_TRIGGER = 1;         //TRIGGER HIGH
            LATGPIO_FLUSH;
            __delay_us(10);                 //10uS Delay 
            ULTRASONIC_TRIGGER = 0;         //TRIGGER LOW
            LATGPIO_FLUSH;
            __delay_ms(ECHO_WAIT);          // WAIT ECHO
            // end measuring disance 
        }
        
        if (distance >= DISTANCE_LIMIT_LOW && distance <= DISTANCE_LIMIT_HIGH  \
                && distance <= DISTANCE_SET) {
            // Check when the result is valid from ultrasonic sensor 
            // and if distance low than value (DISTANCE_SET)
            // can say that now object Present Action
            countActionPresent++;
            countActionEmpty = 0;
        } else {
            //if ultrasonic sensor not answer good value or distance more than
            //(DISTANCE_SET) can say that now object Empty Action
            countActionEmpty++;
            countActionPresent = 0;
        }

        //count Actions try for simulate tiomeout of Actions
        if (countActionPresent >= MAX_COUNT_TRY_PRESENT) {
            RELAY = 1; //RELAY ON 
            countActionPresent = 0;
            TimerStateOff = 0;
        }
        if (countActionEmpty >= MAX_COUNT_TRY_EMPTY) {
            RELAY = 0; //RELAY OFF
            countActionEmpty = 0;
            TimerStateOn = 0;
        }
        
        if (RELAY = 0) {
            TimerStateOff++;
            if (TimerStateOff >= TIME_OFF_DELAY) {
                UltraSonicPower = false;
                TimerStateOff = TIME_OFF_DELAY;
            }
        }

        //checking safety MAX time of State ON
        //when safety timer timeout then sitch off relay in any case
        if ((TimerStateOn >= MAX_TIME_ON)) {
            //general safetimeout
            RELAY = 0; //RELAY OFF
            TimerStateOn = MAX_TIME_ON;
        } else if ((TimerStateOn >= MAX_DOOR_TIME_ON) && (countActionDoor >= MAX_COUNT_TRY_DOOR)) {
            //door safetimeout
            RELAY = 0; //RELAY OFF
            TimerStateOn = MAX_DOOR_TIME_ON;
        } else if (RELAY) {
            //tick for Timer of StateOn
            TimerStateOn++;
        } else {
            TimerStateOn = 0;
        }
        
        LATGPIO_FLUSH; // flush to real GPIO port by all 8 bits
    }
}

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
int8_t countActionDoor;
uint16_t TimerStateOn;
uint8_t TimerStateOff;
bool DoorOpened = false;

//enum  {UltraSonicUndefined, UltraSonicON, UltraSonicOFF};

bool UltraSonicPower = true; //1-on,2-off
bool SafeOffRelay = false; // true if was relay off by safe timer



/******************************************************************************/
/* Main Program                                                               */

/******************************************************************************/

#define DISTANCE_LIMIT_LOW      2   //cm. Object present after this distance
                                    //    Object not present before this distance
#define DISTANCE_LIMIT_HIGH     100 //cm  Object present after this distance
                                    //    Object not present before this distance
#define DISTANCE_SET            60  //cm. Object present before this distance
                                    //    Object not present after this distance
// In General:
// Object not present {0...DISTANCE_LIMIT_LOW} || {DISTANCE_SET...DISTANCE_LIMIT_HIGH} 
// Object present {DISTANCE_LIMIT_LOW...DISTANCE_SET} || {DISTANCE_LIMIT_HIGH...INFINITY}
// This {DISTANCE_LIMIT_HIGH...INFINITY} condition was added after testing, since human body 
// is not good reflective. In some case sound reflect from human body into space
// and not back to sensor.

// Precision measure:
// for 4MHz chip clock, timer use /4 = 1MHz. 
// With this frequency distance precision is 4 cm.


#define TRIGGER_WAIT            10                   //ns
#define ECHO_WAIT               125                  //ms
#define ECHO_WAIT_PER_SEC       1000/ECHO_WAIT       //loops per second

#define MAX_COUNT_TRY_PRESENT   ECHO_WAIT_PER_SEC*1  //seconds
#define MAX_COUNT_TRY_EMPTY     ECHO_WAIT_PER_SEC*15 //seconds
#define MAX_COUNT_TRY_DOOR      ECHO_WAIT_PER_SEC/2  //seconds

#define MINUTES                 60                    //seconds
#define MAX_DOOR_TIME_ON        ECHO_WAIT_PER_SEC*MINUTES*15 //minutes (7200)  u16bit
#define MAX_TIME_ON             ECHO_WAIT_PER_SEC*MINUTES*60 //minutes (28800) u16bit
#define USonicPower_OFF_DELAY   ECHO_WAIT_PER_SEC*MINUTES/2 //minutes  (240)   u8bit

#define USonicPower_on          false
#define USonicPower_off         !USonicPower_on

void main(void) {
    /* Configure the oscillator for the device */
    ConfigureOscillator();

    /* Initialize I/O and Peripherals for application */
    InitApp();

    countActionEmpty = 0;
    countActionPresent = 0;
    countActionDoor = 0;

    ULTRASONIC_TRIGGER = 0; // START POS TRIGGER LOW
    RELAY = 0; // RELAY OFF WHEH POWER ON;
    ULTRASONIC_POWER = USonicPower_off;
    LATGPIO_FLUSH;
    UltraSonicPower = USonicPower_on;

    while (1) {
        CLRWDT();

        #ifndef DEBUG_UART
        if (ULTRASONIC_POWER == !UltraSonicPower) { // only change state
            ULTRASONIC_POWER = UltraSonicPower;
        #else
            ULTRASONIC_POWER = USonicPower_on;
        #endif                
            LATGPIO_FLUSH;
            __delay_us(20); //10uS Delay for start module
        #ifndef DEBUG_UART    
        }
        #endif 

        //check door sensor , opened = 1 , closed = 0
        if (DOOR_SENSOR) {
            countActionDoor++;
        } else {
            countActionDoor--;
        }

        distance = 0; //reset distance

        // checking door sensor
        if (countActionDoor >= MAX_COUNT_TRY_DOOR) {
            //if door opened, will skip measure distance
            countActionPresent = MAX_COUNT_TRY_PRESENT;
            countActionEmpty = 0;
            countActionDoor = MAX_COUNT_TRY_DOOR;
            if (DoorOpened == false) { // detect change state
                DoorOpened = true;
                UltraSonicPower = USonicPower_on;
            }
            __delay_us(TRIGGER_WAIT*10); //10uS x10 Delay 
            __delay_ms(ECHO_WAIT); // SIMULTATE WAIT ECHO
        } else if (countActionDoor <= -MAX_COUNT_TRY_DOOR) {
            // if door closed 
            countActionDoor = -MAX_COUNT_TRY_DOOR;
            // start measure disance 
            if (DoorOpened == true) { // detect change state
                DoorOpened = false;
                SafeOffRelay = false;
                UltraSonicPower = USonicPower_on;
            }

            ULTRASONIC_TRIGGER = 1; //TRIGGER HIGH
            LATGPIO_FLUSH;
            __delay_us(TRIGGER_WAIT); //10uS Delay 
            ULTRASONIC_TRIGGER = 0; //TRIGGER LOW
            LATGPIO_FLUSH;
            __delay_ms(ECHO_WAIT); // WAIT ECHO
            // end measuring disance 

            // here must be result from interrupt with distance set, after delay ECHO_WAIT
            if (distance >= DISTANCE_LIMIT_LOW && (distance <= DISTANCE_SET || distance >= DISTANCE_LIMIT_HIGH)) {
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
        }
        //count Actions try for simulate tiomeout of Actions
        if ((countActionPresent >= MAX_COUNT_TRY_PRESENT) && !SafeOffRelay) {
            RELAY = 1; //RELAY ON 
            countActionPresent = 0;
            TimerStateOff = 0;
        }
        if (countActionEmpty >= MAX_COUNT_TRY_EMPTY) {
            RELAY = 0; //RELAY OFF
            countActionEmpty = 0;
            TimerStateOn = 0;
        }



        //checking safety MAX time of State ON
        //when safety timer timeout then sitch off relay in any case
        if ((TimerStateOn >= MAX_TIME_ON)) {
            //general safe timeout
            RELAY = 0; //RELAY OFF
            TimerStateOn = MAX_TIME_ON;
            SafeOffRelay = true;
        } else if (TimerStateOn >= MAX_DOOR_TIME_ON) {
            if (countActionDoor >= MAX_COUNT_TRY_DOOR) {
                //if door opened is then safe timeout                
                RELAY = 0; //RELAY OFF   
                SafeOffRelay = true;
            }
        }
        if (RELAY) {
            //tick for Timer of StateOn
            TimerStateOn++;
        } else {
            TimerStateOn = 0;
            TimerStateOff++;
            if (TimerStateOff >= USonicPower_OFF_DELAY) {
                UltraSonicPower = USonicPower_off;
                TimerStateOff = USonicPower_OFF_DELAY;
            }
        }
        LATGPIO_FLUSH; // flush to real GPIO port by all 8 bits
        #ifdef DEBUG_UART
        init_serial();
        __delay_us(200); //200uS Delay 
        //send uint16_t format to serial, use RealTerm app for diaplay it
        send_serial_byte(distance>>8);
        send_serial_byte(distance);
        #endif
    }
}


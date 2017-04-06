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
/* Main Program                                                               */
/******************************************************************************/

#define DISTANCE_LIMIT_LOW      2   //cm. Object present after this distance
//    Object not present before this distance
#define DISTANCE_LIMIT_HIGH     92 //cm  Object present after this distance
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


// ********************  MAIN  ************************

void main(void) {
    /* Configure the oscillator for the device */
    ConfigureOscillator();

    /* Initialize I/O and Peripherals for application */
    InitApp();

    countActionEmpty = 0;
    countActionPresent = 0;
    countActionDoor = 0;
    countSkipBeep = SKEEP_BEPPS;
    di(); //GIE = 0; //Global Interrupt DISABLE (measuring disable) 
    ULTRASONIC_TRIGGER = ULTRASONIC_TRIGGER_OFF; // START POS TRIGGER LOW
    RELAY = Relay_off; // RELAY OFF WHEH POWER ON;
    ULTRASONIC_POWER = USonicPower_off;
    BUZZER = BUZZER_OFF;
    LATGPIO_FLUSH;

    UltraSonicPower = USonicPower_on;
    checkUltraSonicPowerforApply();

    HumanBodyWasPressed = false;
    bool PreviosStateLED;
    bool NeedMeasureDistance = false;
    bool NeedFlashAndBuzze = false;

    TimerStateOn = 0;
    TimerStateOff = 0;

    while (1) {

        //check door sensor , opened = 1 , closed = 0
        if (DOOR_SENSOR == DOOR_OPENED) {
            countActionDoor++;
        } else {
            countActionDoor--;
        }

        distance = 0; //reset distance
        NeedMeasureDistance = false;

        // checking door sensor
        if (countActionDoor >= MAX_COUNT_TRY_DOOR) {
            // door opened
            countActionPresent = MAX_COUNT_TRY_PRESENT;
            countActionEmpty = 0;
            countActionDoor = MAX_COUNT_TRY_DOOR;
            if (DoorOpened == false) { // detect change state
                DoorOpened = true;
                //will skip measure distance
                UltraSonicPower = USonicPower_off; //if door opened measuring not required
                HumanBodyWasPressed = false; //reset humman body button state                
                HUMAN_BUTTON_LED = HUMAN_BUTTON_LED_OFF;
                checkUltraSonicPowerforApply();
            }

        } else if (countActionDoor <= -MAX_COUNT_TRY_DOOR) {
            //  door closed 
            countActionDoor = -MAX_COUNT_TRY_DOOR;
            // start measure distance 
            if (DoorOpened == true) { // detect change state
                DoorOpened = false;
                SafeOffRelay = false;
                UltraSonicPower = USonicPower_on;
                checkUltraSonicPowerforApply();
            }
        }

        //DOOR CLOSED
        if (DoorOpened == false) {
            //if UltraSonicPower is off skip most of measuring operation
            if (UltraSonicPower == USonicPower_on) {
                //check if human body button was pressed
                if (!HumanBodyWasPressed) {
                    CLRWDT();
                    PreviosStateLED = HUMAN_BUTTON_LED;
                    HUMAN_BUTTON_TRISBIT = TRISIO_MODE_INPUT;
                    HUMAN_BUTTON_WPU = 1; //WPU PULL-UP
                    __delay_us(5);
                    if (HUMAN_BUTTON == HUMAN_BUTTON_PRESSED) {
                        __delay_us(25); //simple debounce button 
                        if (HUMAN_BUTTON == HUMAN_BUTTON_PRESSED) {
                            HumanBodyWasPressed = true;
                        }
                    }
                    //recover state of pin
                    HUMAN_BUTTON_TRISBIT = TRISIO_MODE_OUTPUT;
                    HUMAN_BUTTON_LED = PreviosStateLED;
                    LATGPIO_FLUSH;
                }
                //start HumanBodyWasPressed ?
                if (HumanBodyWasPressed) {
                    distance = DISTANCE_LIMIT_LOW; // simulate measured disatnce as present
                    countActionPresent = MAX_COUNT_TRY_PRESENT;
                    countActionEmpty = 0;
                    RELAY = Relay_on; //RELAY ON
                    //LED ILUMINATION
                    HUMAN_BUTTON_LED = HUMAN_BUTTON_LED_ON;
                    LATGPIO_FLUSH;
                    UltraSonicPower = USonicPower_off;
                    checkUltraSonicPowerforApply();
                } else if (countActionEmpty) {
                    NeedFlashAndBuzze = true;
                }
                //end HumanBodyWasPressed ?
            }

            NeedMeasureDistance = UltraSonicPower == USonicPower_on && !HumanBodyWasPressed;

            if (NeedFlashAndBuzze) {
                //LED ON - BUZZER - LED OFF
                //HumanBody not Pressed  and empty
                //LED state if Empty state was at least once detected

                NeedFlashAndBuzze = false;
                HUMAN_BUTTON_LED = HUMAN_BUTTON_LED_ON;
                LATGPIO_FLUSH;

                //BUZZER
                if (RELAY == Relay_on) {
                    countSkipBeep--;
                    if (countSkipBeep <= 0) {
                        countSkipBeep = SKEEP_BEPPS;
                        for (int i = countActionEmpty; i >= MAX_COUNT_TRY_EMPTY_BEPPS; i = i - MAX_COUNT_TRY_EMPTY_BEPPS) {
                            CLRWDT();
                            BUZZER = BUZZER_ON;
                            LATGPIO_FLUSH;
                            __delay_ms(7); // beep ON
                            BUZZER = BUZZER_OFF;
                            LATGPIO_FLUSH;
                            __delay_ms(180); // WAIT BEEP off 
                        }
                    } else {
                        __delay_ms(15); // WAIT FOR LED
                    }
                } else {
                    __delay_ms(15); // WAIT FOR LED
                }
                CLRWDT();
                HUMAN_BUTTON_LED = HUMAN_BUTTON_LED_OFF;
                LATGPIO_FLUSH;
            }

            // try start measuring distance
            if (NeedMeasureDistance) {
                PreviosStateLED = HUMAN_BUTTON_LED;
                ULTRASONIC_TRIGGER = ULTRASONIC_TRIGGER_ON; //TRIGGER HIGH
                LATGPIO_FLUSH;
                __delay_us(TRIGGER_WAIT); //10uS Delay 
                ei(); //GIE = 1; //Global Interrupt ENABLE (measuring enable)                
                ULTRASONIC_TRIGGER = ULTRASONIC_TRIGGER_OFF; //TRIGGER LOW
                LATGPIO_FLUSH;
                CLRWDT();
                __delay_ms(ECHO_WAIT); // WAIT ECHO
                //WDT_SLEEP();
                di(); // GIE = 0; //Global Interrupt DISABLE (measuring disable) 
                HUMAN_BUTTON_LED = PreviosStateLED;
                LATGPIO_FLUSH;
            }
            // end measuring distance 

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
        } //DOOR CLOSED


        //count Actions try for simulate timeout of Actions
        if ((countActionPresent >= MAX_COUNT_TRY_PRESENT) && !SafeOffRelay) {
            RELAY = Relay_on; //RELAY ON 
            countActionPresent = 0;
            TimerStateOff = 0;
        } else if (countActionPresent >= MAX_COUNT_TRY_PRESENT) {
            countActionPresent = 0; //for case when SafeOffRelay state is
        }

        if (countActionEmpty >= MAX_COUNT_TRY_EMPTY) {
            RELAY = Relay_off; //RELAY OFF
            countActionEmpty = 0;
            TimerStateOn = 0;
        }

        //checking safety MAX time of State ON
        //when safety timer timeout then switch off relay in any case
        if ((TimerStateOn >= MAX_TIME_ON)) {
            //general safe timeout
            //RELAY = Relay_off; //RELAY OFF
            TimerStateOff = USonicPower_OFF_DELAY; //need power off of USOUND module and LED
            TimerStateOn = MAX_TIME_ON;
            SafeOffRelay = true;
        } else if (TimerStateOn >= MAX_DOOR_TIME_ON) {
            if (countActionDoor >= MAX_COUNT_TRY_DOOR) {
                //if door opened is then safe timeout                
                //RELAY = Relay_off; //RELAY OFF  
                HUMAN_BUTTON_LED = HUMAN_BUTTON_LED_OFF;
                SafeOffRelay = true;
            }
        }

        if (RELAY == Relay_on) {
            //tick for Timer of StateOn
            TimerStateOn++;
        } else {
            TimerStateOn = 0;
            TimerStateOff++;
            if (TimerStateOff >= USonicPower_OFF_DELAY) {
                //power off of USOUND module and LED by timeout
                TimerStateOff = USonicPower_OFF_DELAY;
                UltraSonicPower = USonicPower_off;
                HUMAN_BUTTON_LED = HUMAN_BUTTON_LED_OFF;
                HumanBodyWasPressed = false;
                checkUltraSonicPowerforApply();
            }
        }

        LATGPIO_FLUSH; // flush to real GPIO port by all 8 bits

        if (!NeedMeasureDistance) {
            //delay only if not used measuring was
#if (UseWatchDogForDelay)
            // __delay with watchdog
            OPTION_REGbits.PS = WATCHDOG_PRESCALER_SLEEP; //~144ms 
            SLEEP(); // Included CLRWDT. WAKEUP BY WATCHDOG TIMEOUT  
            NOP();
            //now recover general watchdog time
            OPTION_REGbits.PS = WATCHDOG_PRESCALER_MAIN; //~576ms 
            CLRWDT();
#else
            CLRWDT();
            __delay_ms(ECHO_WAIT); // WAIT ECHO
#endif
        }

#ifdef DEBUG_UART
        init_serial();
        __delay_us(200); //200uS Delay 
        //send uint16_t format to serial, use RealTerm app for display it
        send_serial_byte(distance >> 8);
        send_serial_byte(distance);
#endif
    }
}


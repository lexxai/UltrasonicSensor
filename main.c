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

typedef enum {
    UNDEFINED = 0b00,
    OPENED = 0b01,
    CLOSED = 0b10
} STATES_t;

typedef struct {
    unsigned DOOR : 2;
    unsigned DOOR_CHANGED : 1;
    unsigned TIMERSAFE_ACTIVE : 1;
    unsigned HumanBodyButton_Pressed : 1;
    unsigned NeedFlashAndBuzze : 1;
    unsigned NeedMeasureDistance : 1;
    unsigned MeasuredDistance : 1;
} STATESTORE_t;

void main(void) {
    /* Configure the oscillator for the device */
    ConfigureOscillator();

    /* Initialize I/O and Peripherals for application */
    InitApp();

    countActionEmpty = 0;
    countActionPresent = 0;
    countDebounceDoor = 0;
    countSkipBeep = SKEEP_BEPPS;
    GPIE = 0; //Disble GPIO On-Change Interrupt
    di(); //GIE = 0; //Global Interrupt DISABLE (measuring disable) 
    MEASUREMODULE_TRIGGER = MEASUREMODULE_TRIGGER_OFF; // START POS TRIGGER LOW
    LIGHT_RELAY = Relay_off; // RELAY OFF WHEH POWER ON;
    MEASUREMODULE_POWER = MeasureModulePower_off;
    BUZZER = BUZZER_OFF;
    LATGPIO_FLUSH;

    MeasureModulePower = MeasureModulePower_on;
    checkUltraSonicPowerforApply();

    bool PreviosStateLED;
    bool NeedMeasureDistance = false;
    bool NeedFlashAndBuzze = false;

    STATESTORE_t states;
    states.DOOR = UNDEFINED; //DOOR_STATE_UNDEFINED;
    states.DOOR_CHANGED = 0;
    states.TIMERSAFE_ACTIVE = 0;
    states.NeedFlashAndBuzze = 0;
    states.NeedMeasureDistance = 0;
    states.MeasuredDistance = 0;
    states.HumanBodyButton_Pressed = 0;
    //STATES_t doorState = UNDEFINED; //DOOR_STATE_UNDEFINED;

    TimerStateOn = 0;
    TimerStateOff = 0;


    while (1) {
        CLRWDT();
        states.DOOR_CHANGED = 0;
        states.MeasuredDistance = 0;
        distance = 0; //reset distance
        //CKECKING door sensor , opened = 1 , closed = 0
        if (DOOR_SENSOR == DOOR_OPENED) {
            //wait for debounce of door sensor 
            countDebounceDoor++;
            // checking door sensor debounce state
            if (countDebounceDoor >= MAX_COUNT_TRY_DOOR) {
                // door opened
                countDebounceDoor = MAX_COUNT_TRY_DOOR;
                if (states.DOOR == CLOSED) {
                    states.DOOR_CHANGED = 1;
                }
                states.DOOR = OPENED;
            }
        } else {
            //wait for debounce of door sensor 
            countDebounceDoor--;
            // checking door sensor debounce state
            if (countDebounceDoor <= -MAX_COUNT_TRY_DOOR) {
                //  door closed 
                countDebounceDoor = -MAX_COUNT_TRY_DOOR;
                // detect changed state
                if (states.DOOR == OPENED) {
                    states.DOOR_CHANGED = 1;
                }
                states.DOOR = CLOSED;
            }
        }

        //detect if changed state of door
        if (states.DOOR_CHANGED) { // detect change state
            //reset safe timers
            TimerStateOn = 0;
            TimerStateOff = 0;
            states.TIMERSAFE_ACTIVE = 0;
            states.HumanBodyButton_Pressed = false;
            BUZZER = BUZZER_OFF;
            HUMAN_BUTTON_LED = HUMAN_BUTTON_LED_OFF;
            if (states.DOOR = CLOSED) {
                //Control of Power MeasureModule
                MeasureModulePower = MeasureModulePower_on;
                checkUltraSonicPowerforApply();
            }
        }

        //ACTION depended from DoorState

        if (states.DOOR == OPENED) {
            // door opened
            LIGHT_RELAY = Relay_on; //RELAY On
            //Control of Power MeasureModule
            MeasureModulePower = MeasureModulePower_off;
            checkUltraSonicPowerforApply();
        }

        //------------------------------------------------
        // MEASURING

        if (states.DOOR == CLOSED) {
            // door closed
            if (MeasureModulePower == MeasureModulePower_on) {
                //------------------------------------------
                // Check of pressed BUTTON
                if (!states.HumanBodyButton_Pressed) {
                    CLRWDT();
                    PreviosStateLED = HUMAN_BUTTON_LED;
                    pinMode(HUMAN_BUTTON_TRISBIT, TRISIO_MODE_INPUT);
                    HUMAN_BUTTON_WPU = 1; //WPU PULL-UP
                    __delay_us(5);
                    if (HUMAN_BUTTON == HUMAN_BUTTON_PRESSED) {
                        __delay_us(25); //simple debounce button 
                        if (HUMAN_BUTTON == HUMAN_BUTTON_PRESSED) {
                            states.HumanBodyButton_Pressed = true;
                        }
                    }
                    //recover state of pin
                    pinMode(HUMAN_BUTTON_TRISBIT, TRISIO_MODE_OUTPUT);
                    HUMAN_BUTTON_LED = PreviosStateLED;
                    LATGPIO_FLUSH;
                }


                //--------------------------------------------
                //start states.HumanBodyButton_Pressed ?
                if (states.HumanBodyButton_Pressed) {
                    countActionPresent = MAX_COUNT_TRY_PRESENT;
                    countActionEmpty = 0;
                    LIGHT_RELAY = Relay_on; //RELAY ON
                    //LED ILUMINATION
                    HUMAN_BUTTON_LED = HUMAN_BUTTON_LED_ON;
                    MeasureModulePower = MeasureModulePower_off;
                } else if (countActionEmpty) {
                    states.NeedFlashAndBuzze = true;
                }


            }
        }

        states.NeedMeasureDistance = (MeasureModulePower == MeasureModulePower_on) && !states.HumanBodyButton_Pressed;

        if (states.NeedFlashAndBuzze) {
            states.NeedFlashAndBuzze = false;
            //LED ON - BUZZER - LED OFF
            //HumanBody not Pressed  and empty
            //LED state if Empty state was at least once detected

            //LED ON            
            HUMAN_BUTTON_LED = HUMAN_BUTTON_LED_ON;
            LATGPIO_FLUSH;

            //BUZZER
            if (LIGHT_RELAY == Relay_on) {
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

            //LED OFF
            CLRWDT();
            HUMAN_BUTTON_LED = HUMAN_BUTTON_LED_OFF;
            LATGPIO_FLUSH;
        }

        // try start measuring distance
        if (states.NeedMeasureDistance) {
            PreviosStateLED = HUMAN_BUTTON_LED;
            MEASUREMODULE_TRIGGER = MEASUREMODULE_TRIGGER_ON; //TRIGGER HIGH
            LATGPIO_FLUSH;
            __delay_us(TRIGGER_WAIT); //10uS Delay 
            MEASUREMODULE_ECHO = 0;
            GPIE = 1; //Enable GP On-Change Interrupt 
            ei(); //GIE = 1; //Global Interrupt ENABLE (measuring enable)                
            MEASUREMODULE_TRIGGER = MEASUREMODULE_TRIGGER_OFF; //TRIGGER LOW
            LATGPIO_FLUSH;
            CLRWDT();
            __delay_ms(ECHO_WAIT); // WAIT ECHO
            states.MeasuredDistance = true;
            //WDT_SLEEP();
            di(); // GIE = 0; //Global Interrupt DISABLE (measuring disable) 
            GPIE = 0; //Disable GP On-Change Interrupt
            HUMAN_BUTTON_LED = PreviosStateLED;
            LATGPIO_FLUSH;
        } else {
            distance = DISTANCE_LIMIT_LOW; // simulate measured disatnce as present
        }
        // end measuring distance 


        // Check result from interrupt with distance set, after delay ECHO_WAIT
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

        //------------------------------------------------
        //Actions depednded from distance
        if ((countActionPresent >= MAX_COUNT_TRY_PRESENT)) {
            countActionPresent = MAX_COUNT_TRY_PRESENT;
            LIGHT_RELAY = Relay_on; //RELAY ON 
        }

        if (countActionEmpty >= MAX_COUNT_TRY_EMPTY) {
            countActionEmpty = MAX_COUNT_TRY_EMPTY;
            LIGHT_RELAY = Relay_off; //RELAY OFF
        }




        //------------------------------------------------
        //Calculate Safety Timers
        if (LIGHT_RELAY == Relay_on) {
            //Relay on is
            //tick for Timer of StateOn
            TimerStateOn++;
            TimerStateOff = 0;
        } else {
            //Relay off is
            TimerStateOn = 0;
            TimerStateOff++;

        }


        //Check MAX values of Safety Timers for Light ON
        if ((TimerStateOn >= MAX_TIME_ON)) {
            //general safe timeout
            TimerStateOn = MAX_TIME_ON;
            states.TIMERSAFE_ACTIVE = 1;
        }

        //Check MAX values of Safety Timers for Measure Module
        if (TimerStateOff >= MEASURE_MODULE_POWER_OFF_DELAY) {
            //power off of USOUND module and LED by timeout
            TimerStateOff = MEASURE_MODULE_POWER_OFF_DELAY;
            states.TIMERSAFE_ACTIVE = 1;
        }

        //Check MAX values of timer state for Light ON 
        // in case if door opened
        if (states.DOOR == OPENED) {
            if (TimerStateOn >= MAX_DOOR_TIME_ON) {
                //if door opened is then safe timeout                
                states.TIMERSAFE_ACTIVE = 1;
            }
        }


        if (states.TIMERSAFE_ACTIVE) {
            LIGHT_RELAY = Relay_off; //RELAY OFF
            HUMAN_BUTTON_LED = HUMAN_BUTTON_LED_OFF;
            MeasureModulePower = MeasureModulePower_off;
            checkUltraSonicPowerforApply();
        }


        //FLUSH ALL LAT PIN VALUES TO REAL PIN
        LATGPIO_FLUSH;

        //MAIN LOOP delay 
        //can be skiped if was measuring with own delay
        if (states.MeasuredDistance == false) {
#if (UseWatchDogForDelay)
            // __delay with watchdog

            CLRWDT();
            OPTION_REGbits.PS = WATCHDOG_PRESCALER_SLEEP; //~144ms 
            SLEEP(); // Included CLRWDT. WAKEUP BY WATCHDOG TIMEOUT  
            NOP();
            //now recover general watchdog time
            OPTION_REGbits.PS = WATCHDOG_PRESCALER_MAIN; //~576ms 
#else
            CLRWDT();
            //now recover general watchdog time
            OPTION_REGbits.PS = WATCHDOG_PRESCALER_MAIN; //~576ms 
            __delay_ms(ECHO_WAIT); // WAIT ECHO
#endif
        }
    }
}//main

#if (0)
while (0) {

    //check door sensor , opened = 1 , closed = 0
    if (DOOR_SENSOR == DOOR_OPENED) {
        countDebounceDoor++;
    } else {
        countDebounceDoor--;
    }

    distance = 0; //reset distance
    NeedMeasureDistance = false;

    // checking door sensor
    if (countDebounceDoor >= MAX_COUNT_TRY_DOOR) {
        // door opened
        countActionPresent = MAX_COUNT_TRY_PRESENT;
        countActionEmpty = 0;
        countDebounceDoor = MAX_COUNT_TRY_DOOR;
        if (DoorOpened == false) { // detect change state
            DoorOpened = true;
            DOORSTATES = DOORSTATES::opened; //DOOR_STATE_OPENED;
            //will skip measure distance
            MeasureModulePower = MeasureModulePower_off; //if door opened measuring not required
            states.HumanBodyButton_Pressed = false; //reset humman body button state                
            HUMAN_BUTTON_LED = HUMAN_BUTTON_LED_OFF;
            checkUltraSonicPowerforApply();
        }

    } else if (countDebounceDoor <= -MAX_COUNT_TRY_DOOR) {
        //  door closed 
        countDebounceDoor = -MAX_COUNT_TRY_DOOR;
        // start measure distance 
        if (DoorOpened == true) { // detect change state
            DoorOpened = false;
            DOORSTATES = DOORSTATES::closed; //DOOR_STATE_CLOSED;
            SafeOffRelay = false;
            MeasureModulePower = MeasureModulePower_on;
            checkUltraSonicPowerforApply();
        }
    }

    //DOOR CLOSED
    if (DOORSTATES == DOOR_STATE_CLOSED) {
        //if UltraSonicPower is off skip most of measuring operation
        if (MeasureModulePower == MeasureModulePower_on) {
            //check if human body button was pressed
            if (!states.HumanBodyButton_Pressed) {
                CLRWDT();
                PreviosStateLED = HUMAN_BUTTON_LED;
                HUMAN_BUTTON_TRISBIT = TRISIO_MODE_INPUT;
                HUMAN_BUTTON_WPU = 1; //WPU PULL-UP
                __delay_us(5);
                if (HUMAN_BUTTON == HUMAN_BUTTON_PRESSED) {
                    __delay_us(25); //simple debounce button 
                    if (HUMAN_BUTTON == HUMAN_BUTTON_PRESSED) {
                        states.HumanBodyButton_Pressed = true;
                    }
                }
                //recover state of pin
                HUMAN_BUTTON_TRISBIT = TRISIO_MODE_OUTPUT;
                HUMAN_BUTTON_LED = PreviosStateLED;
                LATGPIO_FLUSH;
            }
            //start states.HumanBodyButton_Pressed ?
            if (states.HumanBodyButton_Pressed) {
                countActionPresent = MAX_COUNT_TRY_PRESENT;
                countActionEmpty = 0;
                LIGHT_RELAY = Relay_on; //RELAY ON
                //LED ILUMINATION
                HUMAN_BUTTON_LED = HUMAN_BUTTON_LED_ON;
                LATGPIO_FLUSH;
                MeasureModulePower = MeasureModulePower_off;
                checkUltraSonicPowerforApply();
            } else if (countActionEmpty) {
                NeedFlashAndBuzze = true;
            }
            //end states.HumanBodyButton_Pressed ?
        }

        NeedMeasureDistance = (MeasureModulePower == MeasureModulePower_on) && !states.HumanBodyButton_Pressed;

        if (NeedFlashAndBuzze) {
            //LED ON - BUZZER - LED OFF
            //HumanBody not Pressed  and empty
            //LED state if Empty state was at least once detected

            NeedFlashAndBuzze = false;
            HUMAN_BUTTON_LED = HUMAN_BUTTON_LED_ON;
            LATGPIO_FLUSH;

            //BUZZER
            if (LIGHT_RELAY == Relay_on) {
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
            MEASUREMODULE_TRIGGER = MEASUREMODULE_TRIGGER_ON; //TRIGGER HIGH
            LATGPIO_FLUSH;
            __delay_us(TRIGGER_WAIT); //10uS Delay 
            ei(); //GIE = 1; //Global Interrupt ENABLE (measuring enable)                
            MEASUREMODULE_TRIGGER = MEASUREMODULE_TRIGGER_OFF; //TRIGGER LOW
            LATGPIO_FLUSH;
            CLRWDT();
            __delay_ms(ECHO_WAIT); // WAIT ECHO
            //WDT_SLEEP();
            di(); // GIE = 0; //Global Interrupt DISABLE (measuring disable) 
            HUMAN_BUTTON_LED = PreviosStateLED;
            LATGPIO_FLUSH;
        } else {
            distance = DISTANCE_LIMIT_LOW; // simulate measured disatnce as present
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
        LIGHT_RELAY = Relay_on; //RELAY ON 
        countActionPresent = 0;
        TimerStateOff = 0;
    } else if (countActionPresent >= MAX_COUNT_TRY_PRESENT) {
        countActionPresent = 0; //for case when SafeOffRelay state is
    }

    if (countActionEmpty >= MAX_COUNT_TRY_EMPTY) {
        LIGHT_RELAY = Relay_off; //RELAY OFF
        countActionEmpty = 0;
        TimerStateOn = 0;
    }

    //checking safety MAX time of State ON
    //when safety timer timeout then switch off relay in any case
    if ((TimerStateOn >= MAX_TIME_ON)) {
        //general safe timeout
        LIGHT_RELAY = Relay_off; //RELAY OFF
        TimerStateOff = MEASURE_MODULE_POWER_OFF_DELAY; //need power off of USOUND module and LED
        TimerStateOn = MAX_TIME_ON;
        SafeOffRelay = true;
    } else if (TimerStateOn >= MAX_DOOR_TIME_ON) {
        if (DOORSTATES == DOOR_STATE_OPENED) {
            //if door opened is then safe timeout                
            LIGHT_RELAY = Relay_off; //RELAY OFF  
            HUMAN_BUTTON_LED = HUMAN_BUTTON_LED_OFF;
            SafeOffRelay = true;
        }
    }

    if (LIGHT_RELAY == Relay_on) {
        //Relay on is
        //tick for Timer of StateOn
        TimerStateOn++;
    } else {
        //Relay off is
        TimerStateOn = 0;
        TimerStateOff++;
        if (TimerStateOff >= MEASURE_MODULE_POWER_OFF_DELAY) {
            //power off of USOUND module and LED by timeout
            TimerStateOff = MEASURE_MODULE_POWER_OFF_DELAY;
            MeasureModulePower = MeasureModulePower_off;
            HUMAN_BUTTON_LED = HUMAN_BUTTON_LED_OFF;
            states.HumanBodyButton_Pressed = false;
            checkUltraSonicPowerforApply();
        }
    }

    LATGPIO_FLUSH; // flush to real GPIO port by all 8 bits

    if (!NeedMeasureDistance) {//
        //delay only if not used measuring was
#if (UseWatchDogForDelay)
        // __delay with watchdog
        CLRWDT();
        OPTION_REGbits.PS = WATCHDOG_PRESCALER_SLEEP; //~144ms 
        SLEEP(); // Included CLRWDT. WAKEUP BY WATCHDOG TIMEOUT  
        NOP();
        //now recover general watchdog time
        OPTION_REGbits.PS = WATCHDOG_PRESCALER_MAIN; //~576ms 
        CLRWDT();
#else
        CLRWDT();
        //now recover general watchdog time
        OPTION_REGbits.PS = WATCHDOG_PRESCALER_MAIN; //~576ms 
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
#endif
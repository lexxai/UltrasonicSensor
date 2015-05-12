/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#if defined(__XC)
    #include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
    #include <htc.h>        /* HiTech General Include File */
#endif

#include <stdint.h>         /* For uint8_t definition */
#include <stdbool.h>        /* For true/false definition */

#include "user.h"





/******************************************************************************/
/* User Functions                                                             */
/******************************************************************************/

/* <Initialize variables in user.h and insert code for user algorithms.> */

void InitApp(void)
{
    /* TODO Initialize User Ports/Peripherals/Project here */

ANSEL = 0;                      //Digital IN

    
ULTRASONIC_TRIGGER_TRISBIT = 0; //OUT 
ULTRASONIC_ECHO_TRISBIT = 1;    //IN

DOOR_SENSOR_TRISBIT = 1;        //IN
RELAY_TRISBIT = 0;              //OUT



    /* Setup analog functionality and port direction */

    /* Initialize peripherals */

    /* Enable interrupts */
    
T1CONbits.TMR1GE=0;     // disable timer1 gate.
T1CONbits.TMR1CS=0;     // internal clock Fosc/4. Fosc/4= 4MHz/4 = 1Mhz
T1CONbits.T1CKPS=0x00;  // prescaler timer1 1:1. 1:1 = 1Mhz
T1CONbits.T1OSCEN=0;    // disable LP.
T1CONbits.TMR1ON=0;     // disable timer1.

ULTRASONIC_TRIGGER = 0;

}


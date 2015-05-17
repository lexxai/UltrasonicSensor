/******************************************************************************/
/*Files to Include                                                            */
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
/* Interrupt Routines                                                         */
/******************************************************************************/

/* Baseline devices don't have interrupts. Unfortunately the baseline detection 
 * macro is named _PIC12 */

#ifndef _PIC12

extern volatile int distance;

void interrupt isr(void) {
    /* This code stub shows general interrupt handling.  Note that these
    conditional statements are not handled within 3 seperate if blocks.
    Do not use a seperate if block for each interrupt flag to avoid run
    time errors. */

#if 1
    /* TODO Add interrupt routine code here. */
    if (GPIF == 1) //Makes sure that it is GP On-Change Interrupt
    {
        GPIE = 0; //Disable On-Change Interrupt
        if (ULTRASONIC_ECHO == 1) //If ECHO is HIGH
            TMR1H = 0;                         //Setting Initial Value of Timer
            TMR1L = 0;                         //Setting Initial Value of Timer            
            TMR1ON = 1; //Start Timer
        if (ULTRASONIC_ECHO == 0) //If ECHO is LOW
        {
            TMR1ON = 0; //Stop Timer
            distance = (TMR1L | (TMR1H << 8)) / 58; //Calculate Distance
        }
    }
    GPIF = 0; //Clear GP On-Change Interrupt flag
    GPIE = 1; //Enable GP On-Change Interrupt
#endif

}
#endif



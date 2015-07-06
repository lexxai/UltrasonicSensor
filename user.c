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

#include "system.h"
#include "user.h"






/******************************************************************************/
/* User Functions                                                             */
/******************************************************************************/

/* <Initialize variables in user.h and insert code for user algorithms.> */

void InitApp(void) {
    /* TODO Initialize User Ports/Peripherals/Project here */
    
    /* Setup analog functionality and port direction */
    ANSEL = 0; //Digital IN

    ULTRASONIC_TRIGGER_TRISBIT = 0; //OUT 
    ULTRASONIC_ECHO_TRISBIT = 1; //IN

    DOOR_SENSOR_TRISBIT = 1; //IN
    RELAY_TRISBIT = 0; //OUT
    
    ULTRASONIC_POWER_TRISBIT  = 0; //OUT  
    #ifdef DEBUG_UART
    UART_OUT_TRISBIT  = 0; //OUT
    #endif
    

    /* Initialize peripherals */
    
   //WEAK PULL-UP for door sensor
    WPU = 0;               //Pull-up disable for all ports
    DOOR_SENSOR_WPU = 1;   //Pull-up enabled for door sensor port (reed switch)
    OPTION_REGbits.nGPPU = 0; // enable individual pull-ups

    
    /* Initialize TIMER 1 */
    
    T1CONbits.TMR1GE = 0; // disable timer1 gate.
    T1CONbits.TMR1CS = 0; // internal clock Fosc/4. Fosc/4= 4MHz/4 = 1Mhz
    T1CONbits.T1CKPS = 0x00; // prescaler timer1 1:1. 1:1 = 1Mhz
    T1CONbits.T1OSCEN = 0; // disable LP.
    T1CONbits.TMR1ON = 0; // disable timer1.
    
    /* Initialize TIMER 0 PRESCASLER FOR WATCHDOG */ 
    OPTION_REGbits.PSA =  1;       // Use internal Watchdog timer ~18ms
    OPTION_REGbits.PS  =  0b101;   // WDT rate 1:32, ~576ms
    
    #ifdef DEBUG_UART
    //Timer 0
    OPTION_REGbits.PSA =  1; // Use TMR0 counting FOSC/4
    OPTION_REGbits.T0CS = 0; // Select TMR0 in Timer Mode (counting FOSC/4)
    #endif

    /* Enable interrupts */
    GPIF = 0; //Clear GPIO On-Change Interrupt Flag
    IOC = ULTRASONIC_ECHO_MASK; //Enable On-Change Interrupt GPIO for ULTRASONIC_ECHO 
    GPIE = 1; //Enable GPIO On-Change Interrupt
    GIE = 1; //Global Interrupt Enable
    
    #ifdef DEBUG_UART
    init_serial();
    #endif
}

#ifdef DEBUG_UART
void init_serial() {
    UART_OUT = SER_BIT; // make hi level
    LATGPIO_FLUSH;
}

void send_serial_byte(unsigned char data) {
    unsigned char i;
    i = 8; // 8 data bits to send

    //PortBShadow &= (!SER_BIT)<<TxPin
    //PORTB = PortBShadow;
    UART_OUT = !SER_BIT; // make start bit
    LATGPIO_FLUSH;
    TMR0 = (256 - SER_BAUD - 5); // load TMR0 value for first baud;
    while (TMR0 & (1 << 7)); // wait for baud

    while (i) // send 8 serial bits, LSB first
    {
        if (data & 1 << 0) UART_OUT = SER_BIT; // send data bit
        else UART_OUT = !SER_BIT;
        LATGPIO_FLUSH;
        data = (data >> 1); // rotate right to get next bit
        i--;
        TMR0 -= SER_BAUD; // load corrected baud value
        while (TMR0 & 1 << 7); // wait for baud
    }
    UART_OUT = SER_BIT; // make stop bit
    LATGPIO_FLUSH;
    TMR0 -= SER_BAUD; // wait a couple of baud for safety
    while (TMR0 & 1 << 7);
    TMR0 -= SER_BAUD; // wait a couple of baud for safety
    while (TMR0 & 1 << 7);
}
void send_serial_byte2(unsigned char data) {
    di();
    unsigned char i;
    i = 8; // 8 data bits to send

    //PortBShadow &= (!SER_BIT)<<TxPin
    //PORTB = PortBShadow;
    UART_OUT = !SER_BIT; // make start bit
    LATGPIO_FLUSH;
#define HALF_BIT_DELAY 44
#define BIT_DELAY 91
    _delay(BIT_DELAY);
    //while (TMR0 & (1 << 7)); // wait for baud

    while (i) // send 8 serial bits, LSB first
    {
        if (data & 1 << 0) UART_OUT = SER_BIT; // send data bit
        else UART_OUT = !SER_BIT;
        data = (data >> 1); // rotate right to get next bit
        i--;
        //TMR0 -= SER_BAUD; // load corrected baud value
        //while (TMR0 & 1 << 7); // wait for baud
        LATGPIO_FLUSH;        
        _delay(BIT_DELAY);
    }
    UART_OUT = SER_BIT; // make stop bit
    LATGPIO_FLUSH;
    _delay(BIT_DELAY);
    ei();
}

#endif

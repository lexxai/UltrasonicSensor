/******************************************************************************/
/* System Level #define Macros                                                */
/******************************************************************************/

/* TODO Define system operating frequency */

/* Microcontroller MIPs (FCY) */
#define SYS_FREQ        4000000L
#define FCY             SYS_FREQ/4
#define _XTAL_FREQ      SYS_FREQ

//#define DEBUG_UART 
#ifdef DEBUG_UART
#define BAUDRATE    9600        // Desierd BAUD Rate (tested) Rates 9600 and 19200
#define SER_BAUD    (((_XTAL_FREQ / 4) / BAUDRATE) - 3) //3
#define HALF_SER_BAUD   SER_BAUD/8
#define SER_BIT     1             // Signal MODE - 1 = Normal 0 = Inverted (Use Inverted for direct 232)
#endif


/******************************************************************************/
/* System Function Prototypes                                                 */
/******************************************************************************/

/* Custom oscillator configuration funtions, reset source evaluation
functions, and other non-peripheral microcontroller initialization functions
go here. */

void ConfigureOscillator(void); /* Handles clock switching/osc initialization */

/******************************************************************************/
/* User Level #define Macros                                                  */
/******************************************************************************/
volatile union {
    unsigned char byte;
    GPIObits_t bits;
} LATGPIO;

#define LATGPIO_FLUSH GPIO = LATGPIO.byte

#define ULTRASONIC_POWER                LATGPIO.bits.GP4
#define ULTRASONIC_POWER_BIT            GP4_bit
#define ULTRASONIC_POWER_TRISBIT        TRISIObits.TRISIO4

#define ULTRASONIC_TRIGGER              LATGPIO.bits.GP5
#define ULTRASONIC_TRIGGER_BIT          GP5_bit
#define ULTRASONIC_TRIGGER_TRISBIT      TRISIObits.TRISIO5

#define ULTRASONIC_ECHO                 GP3
#define ULTRASONIC_ECHO_BIT             GP3_bit
#define ULTRASONIC_ECHO_MASK            _GPIO_GP3_MASK
#define ULTRASONIC_ECHO_TRISBIT         TRISIObits.TRISIO3

#define DOOR_SENSOR                     GP2
#define DOOR_SENSOR_BIT                 GP2_bit
#define DOOR_SENSOR_TRISBIT             TRISIObits.TRISIO2
#define DOOR_SENSOR_WPU                 WPU2

#define RELAY                           LATGPIO.bits.GP1
#define RELAY_BIT                       GP1_bit
#define RELAY_TRISBIT                   TRISIObits.TRISIO1

#ifdef DEBUG_UART
#define UART_OUT                        LATGPIO.bits.GP0
#define UART_OUT_BIT                    GP0_bit
#define UART_OUT_TRISBIT                TRISIObits.TRISIO0
#endif


// Used internal Watchdog timer ~18ms
#define WATCHDOG_PRESCALER_MAIN  0b101  // WDT rate 1:32, ~576ms
#define WATCHDOG_PRESCALER_SLEEP 0b011  // WDT rate 1:8,  ~144ms    

/* TODO Application specific user parameters used in user.c may go here */

/******************************************************************************/
/* User Function Prototypes                                                   */
/******************************************************************************/

void InitApp(void);         /* I/O and Peripheral Initialization */

#ifdef DEBUG_UART
void init_serial();
void send_serial_byte(unsigned char data);
void send_serial_byte2(unsigned char data);
#endif

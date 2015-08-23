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

/* TIMESPECIFIC DEFINITION */
#define TRIGGER_WAIT            10                   //ns
#define ECHO_WAIT               142                  //ms
#define ECHO_WAIT_PER_SEC       1000/ECHO_WAIT       //loops per second

#define MAX_COUNT_TRY_PRESENT   ECHO_WAIT_PER_SEC*1  //seconds
#define MAX_COUNT_TRY_EMPTY     ECHO_WAIT_PER_SEC*15 //seconds
#define MAX_COUNT_TRY_DOOR      ECHO_WAIT_PER_SEC/2  //seconds

#define MINUTES                 60                    //seconds
#define MAX_DOOR_TIME_ON        ECHO_WAIT_PER_SEC*MINUTES*15 //minutes (6300)  u16bit
#define MAX_TIME_ON             ECHO_WAIT_PER_SEC*MINUTES*60 //minutes (25200) u16bit
#define USonicPower_OFF_DELAY   ECHO_WAIT_PER_SEC*MINUTES/2 //minutes  (210)   u8bit
/* TIMESPECIFIC DEFINITION */

// Active pin states for output
#define Relay_on                true
#define Relay_off               !Relay_on

#define USonicPower_on          false
#define USonicPower_off         !USonicPower_on


#define UseWatchDogForDelay      true
// Used internal Watchdog timer ~18ms
#define WATCHDOG_PRESCALER_MAIN  0b101  // WDT rate 1:32, ~576ms
#define WATCHDOG_PRESCALER_SLEEP 0b011  // WDT rate 1:8,  ~144ms    

/* TODO Application specific user parameters used in user.c may go here */

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

bool UltraSonicPower = true; //1-on,2-off
bool SafeOffRelay = false; // true if was relay off by safe timer

/******************************************************************************/
/* User Function Prototypes                                                   */
/******************************************************************************/

void InitApp(void);         /* I/O and Peripheral Initialization */

#ifdef DEBUG_UART
void init_serial();
void send_serial_byte(unsigned char data);
void send_serial_byte2(unsigned char data);
#endif

void WDT_SLEEP(void);
void checkUltraSonicPowerforApply(void);

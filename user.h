/******************************************************************************/
/* User Level #define Macros                                                  */
/******************************************************************************/
volatile union {
    unsigned char byte;
    GPIObits_t bits;
} LATGPIO;

#define LATGPIO_FLUSH GPIO = LATGPIO.byte

#define MEASUREMODULE_POWER                LATGPIO.bits.GP4
#define MEASUREMODULE_POWER_BIT            GP4_bit
#define MEASUREMODULE_POWER_TRISBIT        TRISIObits.TRISIO4

#define MEASUREMODULE_TRIGGER              LATGPIO.bits.GP5
#define MEASUREMODULE_TRIGGER_BIT          GP5_bit
#define MEASUREMODULE_TRIGGER_TRISBIT      TRISIObits.TRISIO5
#define MEASUREMODULE_TRIGGER_ON           1 
#define MEASUREMODULE_TRIGGER_OFF          !MEASUREMODULE_TRIGGER_ON

#define MEASUREMODULE_ECHO                 GP3
#define MEASUREMODULE_ECHO_BIT             GP3_bit
#define MEASUREMODULE_ECHO_MASK            _GPIO_GP3_MASK
#define MEASUREMODULE_ECHO_TRISBIT         TRISIObits.TRISIO3

#define DOOR_SENSOR                     GP2
#define DOOR_SENSOR_BIT                 GP2_bit
#define DOOR_SENSOR_TRISBIT             TRISIObits.TRISIO2
#define DOOR_SENSOR_WPU                 WPUbits.WPU2

#define DOOR_OPENED                     1

#define LIGHT_RELAY                     LATGPIO.bits.GP1
#define LIGHT_RELAY_BIT                 GP1_bit
#define LIGHT_RELAY_TRISBIT             TRISIObits.TRISIO1

#define HUMAN_BUTTON                    GP5
#define HUMAN_BUTTON_BIT                GP5_bit
#define HUMAN_BUTTON_TRISBIT            TRISIObits.TRISIO5
#define HUMAN_BUTTON_WPU                WPUbits.WPU5
#define HUMAN_BUTTON_PRESSED            0

#define HUMAN_BUTTON_LED                MEASUREMODULE_TRIGGER
#define HUMAN_BUTTON_LED_BIT            HUMAN_BUTTON_BIT
#define HUMAN_BUTTON_LED_TRISBIT        HUMAN_BUTTON_TRISBIT
#define HUMAN_BUTTON_LED_ON             1
#define HUMAN_BUTTON_LED_OFF            !HUMAN_BUTTON_LED_ON

#define BUZZER                           LATGPIO.bits.GP0
#define BUZZER_BIT                       GP0_bit
#define BUZZER_TRISBIT                   TRISIObits.TRISIO0
#define BUZZER_ON                        1
#define BUZZER_OFF                       !BUZZER_ON 


#ifdef DEBUG_UART
#define UART_OUT                        LATGPIO.bits.GP0
#define UART_OUT_BIT                    GP0_bit
#define UART_OUT_TRISBIT                TRISIObits.TRISIO0
#endif

/* TIMESPECIFIC DEFINITION */
#define TRIGGER_WAIT            10                   //ns
#define ECHO_WAIT               142                  //ms
#define ECHO_WAIT_PER_SEC       1000/ECHO_WAIT       //loops per second

#define MAX_COUNT_TRY_PRESENT   ECHO_WAIT_PER_SEC*1  // 1 seconds  (7)   u8bit
#define MAX_COUNT_TRY_EMPTY     ECHO_WAIT_PER_SEC*45 // 45 seconds  (315) u16bit
#define MAX_COUNT_TRY_DOOR      ECHO_WAIT_PER_SEC/3  // 1/3 seconds  (2)   u8bit
#define MAX_COUNT_TRY_EMPTY_BEPPS   MAX_COUNT_TRY_EMPTY/4   //define 4 max beeps for all wait period
#define SKEEP_BEPPS             ECHO_WAIT_PER_SEC*5;    //beeps every 8 sec u8bit

#define MINUTES                 60                    //seconds
#define MAX_DOOR_TIME_ON        ECHO_WAIT_PER_SEC*MINUTES*15 // 15 minutes (6300)  u16bit
#define MAX_TIME_ON             ECHO_WAIT_PER_SEC*MINUTES*60 // 60 minutes (25200) u16bit
#define MEASURE_MODULE_POWER_OFF_DELAY   ECHO_WAIT_PER_SEC*MINUTES    // 1 minutes  (420)   u16bit
/* TIMESPECIFIC DEFINITION */

// Active pin states for output
#define Relay_on                true
#define Relay_off               !Relay_on

#define MeasureModulePower_on          false
#define MeasureModulePower_off         !MeasureModulePower_on


#define UseWatchDogForDelay      1
// Used internal Watchdog timer ~18ms
#define WATCHDOG_PRESCALER_MAIN  0b101  // WDT rate 1:32, ~576ms
#define WATCHDOG_PRESCALER_SLEEP 0b011  // WDT rate 1:8,  ~144ms  


#define  TRISIO_MODE_OUTPUT  0 //OUT 
#define  TRISIO_MODE_INPUT   1 //IN 

#define WPU_MODE_ON          1
#define WPU_MODE_OFF         0

#define pinMode(TRISBIT, TRISIO_MODE) TRISBIT = TRISIO_MODE
#define pinModePullUP(WPU, WPU_MODE) WPU = WPU_MODE


/* TODO Application specific user parameters used in user.c may go here */

/******************************************************************************/
/* User Global Variable Declaration                                           */
/******************************************************************************/

volatile uint16_t distance;
uint8_t countActionPresent;
uint16_t countActionEmpty;
int8_t countDebounceDoor;
int8_t countSkipBeep;
uint16_t TimerStateOn;
uint16_t TimerStateOff;
bool DoorOpened = false;

bool MeasureModulePower = MeasureModulePower_off; //1-on,2-off
bool SafeOffRelay = false; // true if was relay off by safe timer

bool HumanBodyWasPressed = false; //if human button pressed measuring ignored

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
void checkMeasureModulePowerforApply(void);


// #include "i2c.h"
#include "bitlit.h"

#define PCF8583_ADDRESS     0x51			// PCF8563 device address (8 bit)
#define I2C_BUSS            "/dev/i2c-1"	// i2c buss that the WaveShare Cape uses for the PCF8563
#define PCF8563_REGS		0x0f			// numer of registers on the PCF8563

// control registers write values
#define CNT_REG_1			B8(00000000)	// control/status1, no test modes or POR override
#define CNT_REG_2			B8(00000000)	// control/status2, no alarm/timer flags and interrupts

// time and date register addresses
#define SEC_REG             0x02
#define MIN_REG             0x03
#define HOUR_REG            0x04
#define DAY_REG             0x05
#define WDAY_REG            0x06
#define MON_REG             0x07
#define YEAR_REG            0x08

// alarm registers write values
#define ALM_REG_MIN			B8(10000000)	// minute alarm values reset to 00
#define ALM_REG_HOUR		B8(10000000)	// hour alarm values reset to 00
#define ALM_REG_DAY			B8(10000000)	// day alarm values reset to 00
#define ALM_REG_WDAY		B8(10000000)	// week day alarm values reset to 00

// CLKOUT control register
#define CLKOUT_REG			B8(00000000)	// CLKOUT outputs inhibited

// timer registers
#define TIMER_REG_1			B8(00000000)	// timer swwiteched off
#define TIMER_REG_2			B8(00000000)	// no timer value

// register masks
#define SEC_MASK            B8(01111111)
#define MIN_MASK            B8(01111111)
#define HOUR_MASK           B8(00111111)
#define DAY_MASK            B8(00111111)
#define WDAY_MASK           B8(00000111)
#define WDAY_MASK           B8(00000111)
#define MON_MASK            B8(00001111)
#define YEAR_MASK           B8(11111111)
#define CENTRY_MASK			B8(10000000)

typedef struct {
    int tm_sec;         /* seconds */
    int tm_min;         /* minutes */
    int tm_hour;        /* hours */
    int tm_mday;        /* day of the month */
    int tm_mon;         /* month */
    int tm_year;        /* year */
    int tm_wday;        /* day of the week */
    int tm_yday;        /* day in the year */
    int tm_isdst;       /* daylight saving time */
} _tm;


int open_tm(char *filename, uint8_t addr);
int get_tm(int, _tm *);
int set_tm(int, _tm *);
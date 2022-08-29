#ifndef __HWSETUPUTILS_H_
#define __HWSETUPUTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MASTER
#include "port_common.h"
#endif

#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "wizchip_conf.h"
#include "w5x00_spi.h"
#include "w5x00_lwip.h"

#include "timer_interface.h"
#include "timer.h"


//100khz
#define I2C_BAUDRATE 100000

// i2c pin number
#define I2C_SLAVE_SDA_PIN 26
#define I2C_SLAVE_SCL_PIN 27

#define I2C_MASTER_SDA_PIN 26
#define I2C_MASTER_SCL_PIN 27

//i2c number for the slave
#define I2C_SLAVE i2c1
#define I2C_MASTER i2c1

//on board led pin number
#define LED_PIN 25
#define LED_PIN_OUT 20

//address pin number
#define I2C_SLAVE_ADDRESS_PIN0 16
#define I2C_SLAVE_ADDRESS_PIN1 17
#define I2C_SLAVE_ADDRESS_PIN2 18
#define I2C_SLAVE_ADDRESS_PIN3 19


#define DEFAULT_SLAVE_ADDRESS 0x50

/* Clock */
#define PLL_SYS_KHZ (133 * 1000)



/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */

/* Clock */
void set_clock_khz(void);

//hardware setup
void hardwareInit();

//utils
uint8_t getAddress();
void onBoardLedToggle();
void onBoardLedOff();
void onBoardLedOn();
void outsideLedToggle();
void  outsideLedOn();
void outsideLedOff();

// uint8_t str_to_array(uint8_t * str, uint8_t * hex);

#ifdef __cplusplus
}
#endif

#endif

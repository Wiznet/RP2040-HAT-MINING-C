#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hwSetupUtils.h"
#include "hardware/spi.h"
#include "hardware/clocks.h"
// #include "timer_interface.h"
/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */

/* Clock */
#define PLL_SYS_KHZ (133 * 1000)

void set_clock_khz(void);
uint8_t str_to_array(uint8_t * str, uint8_t * hex);

void hardwareInit(){
    
#ifndef MASTER
    //gpio setup for i2c address
    gpio_init(I2C_SLAVE_ADDRESS_PIN0);
    gpio_init(I2C_SLAVE_ADDRESS_PIN1);
    gpio_init(I2C_SLAVE_ADDRESS_PIN2);
    gpio_init(I2C_SLAVE_ADDRESS_PIN3);
    
    gpio_set_dir(I2C_SLAVE_ADDRESS_PIN0, GPIO_IN);
    gpio_set_dir(I2C_SLAVE_ADDRESS_PIN1, GPIO_IN);
    gpio_set_dir(I2C_SLAVE_ADDRESS_PIN2, GPIO_IN);
    gpio_set_dir(I2C_SLAVE_ADDRESS_PIN3, GPIO_IN);

    //gpio and i2c function setup
    gpio_init(I2C_SLAVE_SDA_PIN);
    gpio_set_function(I2C_SLAVE_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SDA_PIN);
    
    gpio_init(I2C_SLAVE_SCL_PIN);
    gpio_set_function(I2C_SLAVE_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SCL_PIN);

    i2c_init(I2C_SLAVE, I2C_BAUDRATE);
#else
    gpio_init(I2C_MASTER_SDA_PIN);
    gpio_set_function(I2C_MASTER_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_MASTER_SDA_PIN);
    
    gpio_init(I2C_MASTER_SCL_PIN);
    gpio_set_function(I2C_MASTER_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_MASTER_SCL_PIN);

    i2c_init(I2C_MASTER, I2C_BAUDRATE);
#endif

    //gpio setup for LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    //gpio setup for LED outside the board
    gpio_init(LED_PIN_OUT);
    gpio_set_dir(LED_PIN_OUT,GPIO_OUT);

}

/* Only for the slave */
/* getting the i2c address from the GPIO */
uint8_t getAddress(){
#ifndef MASTER
    uint8_t tempAddress = 0;
    tempAddress = (gpio_get(I2C_SLAVE_ADDRESS_PIN0) << 0) |
    (gpio_get(I2C_SLAVE_ADDRESS_PIN1) << 1) |
    (gpio_get(I2C_SLAVE_ADDRESS_PIN2) << 2) |
    (gpio_get(I2C_SLAVE_ADDRESS_PIN3) << 3);
    
    //if there is something wrong with address read from GPIO, will return 1;
    return ((tempAddress > 0) && (tempAddress < 128)) ? DEFAULT_SLAVE_ADDRESS|tempAddress : DEFAULT_SLAVE_ADDRESS;
#else
    printf("Only for the slave");
#endif
}

/* Common */
void onBoardLedToggle(){
    gpio_put(LED_PIN, !gpio_get(LED_PIN));
}

void onBoardLedOff(){
    gpio_put(LED_PIN, 0);
}

void onBoardLedOn(){
    gpio_put(LED_PIN, 1);
}

/* Only for the slave */
void outsideLedToggle(){
#ifndef MASTER
    gpio_put(LED_PIN_OUT, !gpio_get(LED_PIN_OUT));
#else
    printf("Only for the slave\n");
#endif
}

void  outsideLedOn(){
#ifndef MASTER
    gpio_put(LED_PIN_OUT, 0);//pin low to turn on the led
#else
    printf("Only for the slave");
#endif
}

void outsideLedOff(){
#ifndef MASTER
    gpio_put(LED_PIN_OUT, 1);//pin high to turn off the led
#else
    printf("Only for the slave");
#endif
}

/* Clock */
void set_clock_khz(void)
{
    // set a system clock frequency in khz
    set_sys_clock_khz(PLL_SYS_KHZ, true);

    // configure the specified clock
    clock_configure(
        clk_peri,
        0,                                                // No glitchless mux
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, // System PLL on AUX mux
        PLL_SYS_KHZ * 1000,                               // Input frequency
        PLL_SYS_KHZ * 1000                                // Output (must be same as no divider)
    );
}

// uint8_t str_to_array(uint8_t * str, uint8_t * hex)
// {
// 	uint8_t i;
// 	uint8_t hexcnt = 0;
// 	uint8_t hn, ln;
// 	uint8_t str_tmp[2];
// 	uint8_t len = strlen((char *)str);
	
// 	if((len < 2)  ||  (*str == 0)) return 0;

// 	for(i = 0; i < len; i+=2)
// 	{
// 		//Convert each character to uppercase
// 		str_tmp[0] = (uint8_t)toupper(str[i]);
// 		str_tmp[1] = (uint8_t)toupper(str[i+1]);

// 		hn = str_tmp[0] > '9' ? (str_tmp[0] - 'A' + 10) : (str_tmp[0] - '0');
// 		ln = str_tmp[1] > '9' ? (str_tmp[1] - 'A' + 10) : (str_tmp[1] - '0');

// 		hex[hexcnt++] = (hn << 4) | ln;
// 	}

// 	return 1;
// }
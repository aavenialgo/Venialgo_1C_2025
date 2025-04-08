/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 07/04/2025 | Document creation		                         |
 *
 * @author Andres Venialgo 
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/
typedef struct
{
	gpio_t pin;
	io_t dir;
} gpioConf_t;
/*==================[internal data definition]===============================*/
gpioConf_t gpio_map[4] = {
    {GPIO_20, GPIO_OUTPUT},
    {GPIO_21, GPIO_OUTPUT},
    {GPIO_22, GPIO_OUTPUT},
    {GPIO_23, GPIO_OUTPUT}
};

/*==================[internal functions declaration]=========================*/
void setGpiosFromBcd(uint8_t bcd, gpioConf_t *gpio_vector) {
    /**
     * @brief Set GPIOs based on BCD value
     * @param bcd BCD value to set
     * @param gpio_vector Array of GPIO configurations
     */
    for (int i = 0; i < 4; i++) {
        uint8_t bit = (bcd >> i) & 0x01;
        gpio_write(gpio_vector[i].pin, bit);

        printf("GPIO_%d <- %d (bit %d del BCD)\n", gpio_vector[i].pin, bit, i);

    }
}
/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Hello world!\n");
    GPIOInit(gpio_map[0].pin, gpio_map[0].dir);
    GPIOInit(gpio_map[1].pin, gpio_map[1].dir);
    GPIOInit(gpio_map[2].pin, gpio_map[2].dir);
    GPIOInit(gpio_map[3].pin, gpio_map[3].dir);

    uint8_t bcd_value = 0b1010; // Ejemplo: BCD con bits 0 y 2 en '1'
    setGpiosFromBcd(bcd_value, gpio_map); // Configura los GPIOs según el BCD
}
/*==================[end of file]============================================*/
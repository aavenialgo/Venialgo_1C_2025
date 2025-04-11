/*! @mainpage Template
 *
 * @section genDesc General Description
 * Escribir una función que reciba como parámetro un dígito BCD y 
 * un vector de estructuras del tipo gpioConf_t.
 * Defina un vector que mapee los bits de la siguiente manera:
 *  b0 -> GPIO_20
 *  b1 ->  GPIO_21
 *  b2 -> GPIO_22
 *  b3 -> GPIO_23
 * La función deberá cambiar el estado de cada GPIO, a ‘0’ o a ‘1’, 
 * según el estado del bit correspondiente en el BCD ingresado. 
 * Ejemplo: b0 se encuentra en ‘1’, el estado de GPIO_20 debe setearse. 
 *
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
/**
 * @brief Configura los GPIOs según un valor en formato BCD.
 *
 * @param bcd Valor en formato BCD (4 bits).
 * @param gpio_vector Vector de estructuras `gpioConf_t` que mapea los pines GPIO a los bits del BCD.
 *
 * @note Cambia el estado de cada GPIO a '0' o '1' según el estado del bit correspondiente en el valor BCD.
 *       También imprime en consola el estado de cada GPIO configurado.
 */
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
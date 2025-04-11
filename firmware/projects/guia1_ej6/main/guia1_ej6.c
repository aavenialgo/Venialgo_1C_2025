/*! @mainpage Guia1
 *
 * @section Enunciado del problema:
 Escriba una función que reciba un dato de 32 bits,  la cantidad de dígitos 
 de salida y dos vectores de estructuras del tipo  gpioConf_t. 
 Uno  de estos vectores es igual al definido en el punto anterior 
 y el otro vector mapea los puertos con el dígito del LCD a donde
 mostrar un dato:

 Dígito 1 -> GPIO_19
 Dígito 2 -> GPIO_18
 Dígito 3 -> GPIO_9

 La función deberá mostrar por display el valor que recibe. 
 Reutilice las funciones creadas en el punto 4 y 5
 *
 * @section Proyecto 1
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

gpioConf_t gpio_map_bcd[4] = {
	{GPIO_20, GPIO_OUTPUT},
	{GPIO_21, GPIO_OUTPUT},
	{GPIO_22, GPIO_OUTPUT},
	{GPIO_23, GPIO_OUTPUT}
};
gpioConf_t gpio_map_digits[3] = {
	{GPIO_19, GPIO_OUTPUT},
	{GPIO_18, GPIO_OUTPUT},
	{GPIO_9, GPIO_OUTPUT}
};

/*==================[internal data definition]===============================*/
	
/*==================[internal functions declaration]=========================*/
/**
 * @brief Convierte un número entero en un arreglo de dígitos en formato BCD.
 *
 * @param data Número de 32 bits a convertir.
 * @param n_digits Cantidad de dígitos que se desea obtener.
 * @param bcd_number Puntero al arreglo donde se almacenarán los dígitos en formato BCD.
 * @return int8_t Devuelve 0 si la conversión fue exitosa, -1 si el número tiene más dígitos que los permitidos.
 *
 * @note El arreglo `bcd_number` debe tener suficiente espacio para almacenar `n_digits`.
 */
int8_t convertToBcdArray(uint32_t data, uint8_t n_digits, uint8_t *bcd_number){
	if (n_digits > 10){
		return -1;
	}
	for (int i = 0; i < n_digits; i++){
		bcd_number[n_digits-1-i] = data % 10;
		data /= 10;
	}
	if (data >0){
		return -1; // el numero tiene mas digitos que los permitidos
	}
	return 0;
}
/**
 * @brief Configura los pines GPIO de acuerdo con un valor BCD.
 *
 * @param bcd Valor en formato BCD (4 bits).
 * @param gpio_vector Vector de estructuras `gpioConf_t` que mapea los pines 
 * GPIO a los bits del BCD.
 * @note Enciende o apaga los pines GPIO según los bits del valor BCD.
 */
void setGpiosFromBcd(uint8_t bcd, gpioConf_t *gpio_vector) {
	for (int i = 0; i < 4; i++) {
		uint8_t bit = (bcd >> i) & 0x01;
		if (bit) {
			GPIOOn(gpio_vector[i].pin);
		} else {
			GPIOOff(gpio_vector[i].pin);
		}
	}
}
/**
 * @brief Muestra un valor en un display LCD utilizando pines GPIO.
 * @param value Valor numérico a mostrar.
 * @param num_digits Cantidad de dígitos a mostrar en el display.
 * @param bcd_vector Vector de estructuras `gpioConf_t` que mapea los 
 * pines GPIO para los bits BCD.
 * @param digit_vector Vector de estructuras `gpioConf_t` que mapea 
 * los pines GPIO para los dígitos del display.
 *
 * @note La función convierte el valor a formato BCD y 
 * configura los pines GPIO para mostrar cada dígito en el display.
 */
void displayValueOnLcd(uint32_t value, uint8_t num_digits, gpioConf_t *bcd_vector, gpioConf_t *digit_vector) {
	uint8_t bcd_array[10]; // hasta 10 dígitos máximo
	if (convertToBcdArray(value, num_digits, bcd_array) != 0){
		return;
	}

	for (int i = 0; i < num_digits; i++) {
		setGpiosFromBcd(bcd_array[i], bcd_vector); // Configura los pines BCD
		GPIOOn(digit_vector[i].pin);

		GPIOOff(digit_vector[i].pin); 

		// vTaskDelay(5 / portTICK_PERIOD_MS); // Pequeño retardo de visualización

	}
}
/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Hello world!\n");

    for (int i = 0; i < 4; i++) {
        GPIOInit(gpio_map_bcd[i].pin, gpio_map_bcd[i].dir);
    }
    for (int i = 0; i < 3; i++) {
        GPIOInit(gpio_map_digits[i].pin, gpio_map_digits[i].dir);
    }

    uint32_t value = 123;
    uint8_t num_digits = 3;
    displayValueOnLcd(value, num_digits, gpio_map_bcd, gpio_map_digits);
}
/*==================[end of file]============================================*/
/*! @mainpage Blinking switch modified
 *
 * @section genDesc General Description
 *
 * Proyecto guia 1 ejercicio 2 (blinking switch modificado)
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
 * | 17/09/2023 | Document creation		                         |
 *
 * @author Andres Venialgo
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
/*==================[macros and definitions]=================================*/
bool led1_status = false;
bool led2_status = false;
#define CONFIG_BLINK_PERIOD 1000
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
	uint8_t teclas;
	LedsInit();
	SwitchesInit();
	while (1)
	{
	teclas = SwitchesRead();
	switch(teclas){
		case SWITCH_1:
		led1_status = !led1_status;
		if (led1_status)
		{
			LedOn(LED_1);
		}
		else{
			LedOff(LED_1);
		}
		break;
		case SWITCH_2:
		led2_status = !led2_status;
		if (led2_status)
		{
			LedOn(LED_2);
		}
		else{
			LedOff(LED_2);
		}
		break;
	}
	if ((teclas & SWITCH_1) && (teclas & SWITCH_2)) {
		LedOn(LED_3);
	} else {
		LedOff(LED_3);
	}
	}
	vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
	printf("Hello world!\n");
}
/*==================[end of file]============================================*/
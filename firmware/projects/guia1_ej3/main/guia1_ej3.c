/*! @mainpage Guia1_3
 *
 * @section Ejercicio 3 del proyecto 1
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
 * | 12/09/2023 | Document creation		                         |
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
typedef struct leds
{
      uint8_t mode;     //  ON, OFF, TOGGLE
	uint8_t n_led;      //  indica el número de led a controlar
	uint8_t n_ciclos;   //  indica la cantidad de ciclos de ncendido/apagado
	uint16_t periodo;   //  indica el tiempo de cada ciclo
} my_leds; 

/*==================[internal data definition]===============================*/
#define ON 1 
#define OFF 0 
#define TOGGLE 2 
#define CONFIG_BLINK_PERIOD 1000
/*==================[internal functions declaration]=========================*/
void funcionLeds(my_leds *leds){
	switch (leds->mode)
	{
	case (ON):
			LedOn(leds->n_led);
		break;
	case (OFF):
			LedOff(leds->n_led);
		break;
	case (TOGGLE):
	uint8_t retardo = leds->periodo / leds->n_ciclos;
		for (int i = 0; i < leds->n_ciclos; i++) {
			if (leds->n_led == 1)
				LedToggle(leds->n_led );
			for (int j=0; j < retardo; j++){
				vTaskDelay(CONFIG_BLINK_PERIOD/portTICK_PERIOD_MS);	
		}
	}
}
}

/*==================[external functions definition]==========================*/
void app_main(void){
	LedsInit();
	printf("Hello world!\n");
	my_leds prueba1 = {1, 3, 10, 1000};
	funcionLeds(&prueba1);
}
/*==================[end of file]============================================*/
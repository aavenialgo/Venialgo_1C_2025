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
#define CONFIG_BLINK_PERIOD 100
/*==================[internal functions declaration]=========================*/
/**
 * @brief Controla el comportamiento de un LED según el modo especificado.
 *
 * @param leds Puntero a una estructura `my_leds` que contiene la configuración del LED:
 * - `mode`: Modo de operación del LED (ON, OFF, TOGGLE).
 * - `n_led`: Número del LED a controlar.
 * - `n_ciclos`: Cantidad de ciclos de encendido/apagado (solo para TOGGLE).
 * - `periodo`: Tiempo de duración de cada ciclo en milisegundos (solo para TOGGLE).
 *
 * @note En el modo TOGGLE, el LED alterna entre encendido y apagado durante el número de ciclos especificado,
 *       con un retardo configurado por el período dividido en intervalos de `CONFIG_BLINK_PERIOD`.
 */
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
	uint8_t retardo = leds->periodo / CONFIG_BLINK_PERIOD;
		for (int i = 0; i < leds->n_ciclos; i++) {
			printf("TOGGLE\n");	
			LedToggle(leds->n_led );
			for (int j=0; j < retardo; j++){
				vTaskDelay(CONFIG_BLINK_PERIOD/portTICK_PERIOD_MS);	
		}
	}
		break;
}
}

/*==================[external functions definition]==========================*/
void app_main(void){
	LedsInit();
	printf("Hello world!\n");
	my_leds prueba1 = {TOGGLE, LED_1, 10, 500};
	funcionLeds(&prueba1);
}
/*==================[end of file]============================================*/
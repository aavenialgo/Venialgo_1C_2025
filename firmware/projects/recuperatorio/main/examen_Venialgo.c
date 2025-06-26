/*! @mainpage Recuperatorio de Examen EPROM
 *
 * @section genDesc General Description
 * Consigna de Aplicación: Alimentador automático de mascotas
 * Se pretende diseñar un dispositivo basado en la ESP-EDU que se utilizará para suministrar alimento y agua a una mascota.
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
 * | 26/06/2025 | Document creation		                         |
 *
 * @author andres venialgo 
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "hc_sr04.h"
#include "led.h"
#include "switch.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/
int16_t volumen_agua = 0; 
int16_t peso_comida = 0;

#define CONFIG_MEASURE_NIVEL_AGUA 2000 // TIempo medicion del nivel de agua
#define CONFIG_MEASURE_NIVEL_COMIDA 5000 // Tiempo medicion del nivel de comida 
#define CONFIG_INFORMAR_NIVEL 10000 // Tiempo de informar el nivel de agua y comida 
/*==================[internal data definition]===============================*/
TaskHandle_t task_handle_agua = NULL; // Tarea para controlar el agua
TaskHandle_t task_handle_comida = NULL; // Tarea para controlar la comida
TaskHandle_t task_handle_informar = NULL; 
/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
	
}
/*==================[end of file]============================================*/
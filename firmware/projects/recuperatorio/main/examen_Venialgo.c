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
/*! @brief variable que guarda el volumen de agua en el recipiente */
int16_t volumen_agua = 0; 
/*! @brief booleano para activar/desactivar la medicion del volumen de agua */
bool medir_agua = true;
/*! @brief variable que guarda el peso de la comida*/
int16_t peso_comida = 0;
/*! @brief booleano para activar/desactivar la medicion del peso de la comida */
bool medir_comida = true;
/*! @brief Variable que almacena la tecla leída. */
uint8_t key = 0;


#define CONFIG_MEASURE_NIVEL_AGUA 2000 // TIempo medicion del nivel de agua
#define CONFIG_MEASURE_NIVEL_COMIDA 5000 // Tiempo medicion del nivel de comida 
#define CONFIG_INFORMAR_NIVEL 10000 // Tiempo de informar el nivel de agua y comida 
/*==================[internal data definition]===============================*/
TaskHandle_t task_handle_agua = NULL; // Tarea para controlar el agua
TaskHandle_t task_handle_comida = NULL; // Tarea para controlar la comida
TaskHandle_t task_handle_informar = NULL; 
/*==================[internal functions declaration]=========================*/

static void medirNivelAguaTask(void *pParameter){

}
static void medirNivelComidaTask(void *pParameter){
	
}

static void mandarInformacionTask (void *pParameter){

}
/** @brief Función asociada a la interrupción del switch 1. corresponde a medir/no medir el volumen de agua
 */
void functionKey1(void *pParameter){
	medir_agua = !medir_agua; 
}
/** @brief Función asociada a la interrupción del switch 2. corresponde a medir/no medir el peso de la comida
 */
void functionKey2(void *pParameter){
	medir_comida = !medir_comida;
}

void inicializarPerifericos(){
	LedInit();
	SwitchesInit();
	SwitchActivInt(SWITCH_1, functionKey1, NULL);
	SwitchActivInt(SWITCH_2, functionKey2, NULL);
	HcSr04Init(GPIO_3, GPIO_2); // Trigger y Echo
}
/*==================[external functions definition]==========================*/
void app_main(void){
	inicializarPerifericos();

	xTaskCreate(medirNivelAguaTask, "Medir Nivel Agua", 2048, NULL, 5, &task_handle_agua);
	xTaskCreate(medirNivelComidaTask, "Medir Nivel Comida", 2048, NULL, 5, &task_handle_comida);
	xTaskCreate(mandarInformacionTask, "Informar Nivel", 2048, NULL, 5, &task_handle_informar);

	
}
/*==================[end of file]============================================*/
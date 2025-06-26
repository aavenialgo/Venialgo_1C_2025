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
#define PESO_MINIMO_G 25
#define PESO_MAXIMO_G 250 // Peso maximo de comida que se puede medir
/*==================[internal data definition]===============================*/
/*! @brief Tarea para controlar el nivel de agua */
TaskHandle_t task_handle_agua = NULL; 
/*! @brief Tarea para controlar el nivel de comida */
TaskHandle_t task_handle_comida = NULL; // Tarea para controlar la comida
/*! @brief Tarea para controlar informar*/
TaskHandle_t task_handle_informar = NULL; 


/*==================[internal functions declaration]=========================*/
/** 
 *  @brief Tarea que controla el nivel de agua
 */ 
static void controlNivelAguaTask(void *pParameter){
	float altura_total = 9.5;
	float altura_actual;
	float area_base = 31.57; 

	uint8_t distancia = 0;
	while(true){
		if (medir_agua){
			distancia = HcSr04ReadDistanceInCentimeters();

			altura_actual = altura_total - (distancia / 100.0f); // Convertir a metros
			volumen_agua = altura_actual * area_base * 1000; // Convertir a ml
			if (volumen_agua < 2500) 
				GPIOOn(GPIO_5);
			else 
			GPIOOff(GPIO_5);
	}
	vTaskDelay(CONFIG_MEASURE_NIVEL_AGUA / portTICK_PERIOD_MS);
}
}
/** 
 *  @brief Función que lee el valor del ADC y lo convierte a peso en gramos
 *  @return Peso en gramos
 */
float leerPeso(uint16_t adcValue){
	//suponiendo ADC 12 bits (0-4095) y de referencia de 3.3 V
	float voltaje = (adcValue / 4095.0f) * 3.3f;
    float peso = (voltaje - 0.5f) * (500.0f / (3.3f - 0.5f));
    if (peso < 0) 
	peso = 0;
    if (peso > 500)
	peso = 500;
	return peso
}
/** 
 *  @brief Tarea que controla el nivel de agua
 * @param pParameter Parámetro de entrada (no utilizado).
 */
static void controlNivelComidaTask(void *pParameter){
	uint16_t adcValue = 0;
	uint16_t comida_actual = 0;
	while(true){
		AnalogInputRead(CH1, &adcValue); 
		comida_actual = leerPeso(adcValue); 
		if (medir_comida){
			if (comida_actual < PESO_MINIMO_G) {
				GPIOOn(GPIO_10); 
			} else if (comida_actual > PESO_MAXIMO_G) {
				GPIOOff(GPIO_10); 
			} else {
				GPIOOff(GPIO_10); 
			}
			peso_comida = comida_actual;
		}
		vTaskDelay(CONFIG_MEASURE_NIVEL_COMIDA / portTICK_PERIOD_MS);
    
	}
}

static void mandarInformacionTask (void *pParameter){
	  char mensaje[64];
    const char* nombre_mascota = "Firulais";
    while (1) {
        snprintf(mensaje, sizeof(mensaje), "%s tiene %d g de alimento y %d ml de agua.\r\n", 
                 nombre_mascota, peso_comida, volumen_agua);
        UartSendString(UART_PC, mensaje);
        vTaskDelay(CONFIG_INFORMAR_NIVEL / portTICK_PERIOD_MS);
    }
}

void uartKey(void *param){
	while(true){
		if (UartReadByte(UART_PC, &key)== 0){
			switch(key){
				case 'w': 
					medir_agua = !medir_agua; 
					break;
				case 'f': 
					medir_comida = !medir_comida;
					break;
				default:
				key = 0;
					break;
			}
		}
        
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
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

	analog_input_config_t medicion= {  //senial balanza
    .input = CH1, 
	};
	AnalogInputInit(&medicion); // Configuracion del ADC para medir el nivel de agua y comida
	AnalogOutputInit();

	serial_config_t uart_config = {
		.baud_rate = 115200,
		.port = CH1,
		.func_p = NULL, 
		.param_p = NULL
   };
   UartInit(&uart_config);
}
/*==================[external functions definition]==========================*/
void app_main(void){
	inicializarPerifericos();

	xTaskCreate(controlNivelAguaTask, "Medir Nivel Agua", 2048, NULL, 5, &task_handle_agua);
	xTaskCreate(controlNivelComidaTask, "Medir Nivel Comida", 2048, NULL, 5, &task_handle_comida);
	xTaskCreate(mandarInformacionTask, "Informar Nivel", 2048, NULL, 5, &task_handle_informar);
	xTaskCreate(uartKey, "Leer Tecla", 2048, NULL, 5, NULL);
	
}
/*==================[end of file]============================================*/
/*! @mainpage Prueba para PPG
 *
 * @section genDesc General Description
 * En este proyecto se prueba el funcionamiento de levantar una señal de PPG
 * con un sensor max30102 (o similar).
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
 * | 06/06/2025 | Document creation		                         |
 *
 * @author Andres Venialgo (andres.venialgo@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <iir_filter.h>
#include <max3010x.h>
#include "spo2_algorithm.h"
#include "led.h"
/*==================[macros and definitions]=================================*/
#define BUFFER_SIZE 256
#define SAMPLE_FREQ	100
#define CONFIG_BLINK_PERIOD 100
/*==================[internal data definition]===============================*/
float dato_filt;
float dato;

uint32_t irBuffer[100]; //infrared LED sensor data
uint32_t redBuffer[100];  //red LED sensor data
int32_t bufferLength=100; //data length
int32_t spo2; //SPO2 value
int8_t validSPO2; //indicator to show if the SPO2 calculation is valid
int32_t heartRate; //heart rate value
int8_t validHeartRate; //indicator to show if the heart rate calculation is valid
/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
    /* Filtro pasa altos de orden 4 con frecuencia de corte en 1Hz */
	HiPassInit(SAMPLE_FREQ, 1, ORDER_4);
    LedsInit();
    MAX3010X_begin();
	MAX3010X_setup( 30, 1 , 2, SAMPLE_FREQ, 69, 4096);
    /* Se imprimen por consola los valores de frequencia y magnitud correspondiente */

    while(1){
        uint8_t i;
	    for (i = 25; i < 100; i++)
	    {
		    redBuffer[i - 25] = redBuffer[i];
    	}

	    for ( i = 75; i < 100; i++)
	    {
		    while (MAX3010X_available() == false) //do we have new data?
			    MAX3010X_check(); //Check the sensor for new data
            
		    redBuffer[i] = MAX3010X_getRed();
		    MAX3010X_nextSample(); 
		    
            
            red_val = (float)redValue;
	     	dato = (float)redBuffer[i];
			HiPassFilter(&dato, &dato_filt, 1);
            printf("Dato filtrado: %.2f\n", dato_filt); 
            vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS); 
	    } 
    }
}
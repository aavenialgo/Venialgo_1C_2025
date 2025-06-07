/*! @mainpage Proyecto EPROM
 *
 * @section genDesc General Description
 *	Este es el proyecto integrador de la asignatura electronica programable.
 * El mismo consiste la utilizacion de dos sensores, uno de ppg y otro de 
 * biopotenciales (para levantar un ECG), los cuales se encuentran conectados a 
 * un microcontrolador ESP32.
 * El objetivo del proyecto es la lectura de los datos de ambos sensores y su
 * posterior envio a una aplicacion movil mediante bluetooth.
 * Constara de un procesamiento basico de las señales como filtro para mejorar
 * su calidad.
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
 * | 22/05/2025 | Document creation		                         |
 *
 * @author Andres Venialgo
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "led.h"
#include "timer_mcu.h"
#include "iir_filter.h"
#include "ble_mcu.h"
#include "analog_io_mcu.h"
#include "timer_mcu.h"


//#include "spo2_algorithm.h"
//#include <max3010x.h>

/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 500
#define LED_BT LED_1
#define SAMPLE_FREQ 200
#define T_SENIAL 4000
#define CHUNK 4	
#define PERIODO_REFRACTARIO 250000
#define PERIODO_MUESTREO 10000


uint8_t muestras_en_periodo_refractario = PERIODO_REFRACTARIO / PERIODO_MUESTREO;
int16_t UMBRAL_ONDA_R = 400;
int16_t UMBRAL_INF_FC = 60;
int16_t UMBRAL_SUP_FC = 80;
uint16_t valor_actual = 0;
uint16_t periodo_RR = 0;
TaskHandle_t task_procesar_enviar = NULL;

static float ecg_filt[CHUNK] = {0};
static float ecg_muestra[CHUNK] = {0};

#define BUFFER SIZE 256



/*==================[internal data definition]===============================*/
float ecg[] = {
	76,  76,  77,  77,  76,  83,  85,  78,  76,  85,  93,  85,  79,
	86,  93,  93,  85,  87,  94,  98,  93,  87,  95, 104,  99,  91,
	93, 102, 104,  99,  96, 101, 106, 102,  96,  97, 104, 106,  97,
	94, 100, 103, 101,  91,  95, 103, 100,  94,  90,  98, 104,  94,
	87,  93,  99,  97,  87,  86,  96,  98,  90,  83,  90,  96,  89,
	81,  80,  87,  92,  82,  78,  84,  89,  80,  72,  78,  82,  82,
	73,  72,  81,  82,  79,  69,  77,  82,  81,  76,  68,  78,  80,
	76,  73,  78,  82,  82,  75,  72,  86,  84,  78,  76,  85,  95,
	88,  81,  83,  93,  90,  86,  83,  88,  93,  86,  82,  82,  92,
	89,  82,  82,  88,  94,  84,  82,  90,  98,  94,  87,  91,  95,
	98,  93,  90,  97, 104, 105,  96,  93, 107, 116, 118, 127, 148,
   181, 208, 231, 252, 241, 198, 139,  76,  43,  32,  29,  42,  65,
	86,  90,  88,  93, 101, 107, 102,  98, 103, 110, 104,  98,  99,
   107, 109,  96,  95, 103, 107, 102,  95,  95, 102, 105,  94,  94,
   102, 102,  99,  94,  96, 102,  99,  90,  92, 100, 102,  95,  90,
	98, 104,  97,  89,  94, 102, 103,  97,  93, 100, 105, 102,  93,
	97, 104, 104, 100,  96, 108, 111, 104,  99, 101, 108, 102,  96,
	97, 104, 104,  97,  89,  91, 100,  91,  81,  79,  85,  86,  73,
	69,  75,  79,  75,  68,  68,  76,  76,  69,  67,  74,  81,  77,
	71,  72,  82,  82,  76,  77,  76,  76,  75
};

bool filter = false;
TaskHandle_t task_handle = NULL;


// constantes relacionadas al sensor MAX30102
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
void read_data(uint8_t *data, uint8_t length)
{
	uint8_t i = 1;
	uint16_t umbral_aux = 0;
	char msg[60];
	if (data[0] == 'A')
	{
		strcpy(msg, "");
		umbral_aux = 0;
		while (data[i] != 'B')
		{
			umbral_aux = umbral_aux * 10;
			umbral_aux = umbral_aux + (data[i] - '0');
			i++;
		}
		UMBRAL_ONDA_R = umbral_aux;
		sprintf(msg, "U%u\n", umbral_aux);
		printf("%u\n", umbral_aux);
	}

	if (data[0] == 'C')
	{
		strcpy(msg, "");
		umbral_aux = 0;
		while (data[i] != 'D')
		{
			umbral_aux = umbral_aux * 10;
			umbral_aux = umbral_aux + (data[i] - '0');
			i++;
		}
		UMBRAL_INF_FC = umbral_aux;
		sprintf(msg, "V%u\n", umbral_aux);
		printf("%u\n", umbral_aux);
	}

	if (data[0] == 'E')
	{
		strcpy(msg, "");
		umbral_aux = 0;
		while (data[i] != 'F')
		{
			umbral_aux = umbral_aux * 10;
			umbral_aux = umbral_aux + (data[i] - '0');
			i++;
		}
		UMBRAL_SUP_FC = umbral_aux;
		sprintf(msg, "W%u\n", umbral_aux);
		printf("%u\n", umbral_aux);
	}

	BleSendString(msg);
}


static void procesar_enviar(void *pvParameter)
{
	uint16_t valor = 0;
	char msg[128];
	char msg_chunk[24];
	uint8_t i = 0;
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		AnalogInputReadSingle(CH1, &valor);
		ecg_muestra[i] = valor;
		valor_actual = valor;
		periodo_RR++;
		i++;

		if (i == CHUNK)
		{
			HiPassFilter(&ecg_muestra[0], ecg_filt, CHUNK);
			LowPassFilter(ecg_filt, ecg_filt, CHUNK);
			strcpy(msg, "");

			for (uint8_t j = 0; j < CHUNK; j++)
			{
				
				sprintf(msg_chunk, "*G%.2f*", ecg_filt[j]);
				strcat(msg, msg_chunk);
			}
			BleSendString(msg);
			i = 0;
		}
	}
}
void FuncTimerSenial(void* param){
    xTaskNotifyGive(task_handle);
}
void inicialitePeripherals(){
	LedsInit();
	LowPassInit(SAMPLE_FREQ, 30, ORDER_2);
    HiPassInit(SAMPLE_FREQ, 1, ORDER_2);
	ble_config_t ble_configuration = {
		"chui_esp32c6",
		read_data};
	BleInit(&ble_configuration);

}
/*==================[external functions definition]==========================*/
void app_main(void){

	inicialitePeripherals();
	timer_config_t timer_senial = {
        .timer = TIMER_B,
        .period = T_SENIAL*CHUNK,
        .func_p = FuncTimerSenial,
        .param_p = NULL
    };
	TimerInit(&timer_senial);
    TimerStart(timer_senial.timer);

	analog_input_config_t analog_input = {
		.input = CH1,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL};

	AnalogInputInit(&analog_input);
	AnalogOutputInit();

	// Create the task for filtering data
	xTaskCreate(procesar_enviar, "leo y envio datos", 2048, NULL, 5, &task_handle);

	while(1){
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
        switch(BleStatus()){
            case BLE_OFF:
                LedOff(LED_BT);
            break;
            case BLE_DISCONNECTED:
                LedToggle(LED_BT);
            break;
            case BLE_CONNECTED:
                LedOn(LED_BT);
            break;
        }

	}
}
/*==================[end of file]============================================*/
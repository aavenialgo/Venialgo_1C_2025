/*! @mainpage Proyecto EPROM
 *
 * @section genDesc General Description
 *	Este es el proyecto integrador de la asignatura electronica programable.
 * El mismo consiste la utilizacion de dos sensores, uno de ppg, montado en
 * una placa y otro de biopotenciales (para levantar un ECG), los cuales se 
 * encuentran conectados a un microcontrolador ESP32.
 * El objetivo del proyecto es la lectura de los datos de ambos sensores y su
 * posterior envio a ECG a una script de Pyhton mediante bluetooth y PPG a un puerto serie.
 * Constara de un procesamiento basico de las señales como filtro para mejorar
 * su calidad.
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * |   ECG Input    |    GPIO_02    |
 * |   PPG Input    |    GPIO_01    |
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
#include "uart_mcu.h"

/*==================[macros and definitions]=================================*/
/*! @brief Periodo de parpadeo del LED en milisegundos. */
#define CONFIG_BLINK_PERIOD 500
/*! @brief LED utilizado para indicar el estado del Bluetooth. */
#define LED_BT LED_1
/*! @brief Frecuencia de muestreo del ECG en Hz. */
#define SAMPLE_FREQ 200
/*! @brief Periodo de muestreo del ECG en microsegundos. */
#define T_SENIAL 4000
/*! @brief Tamaño del chunk de datos a procesar. */
#define CHUNK 8
/*! @brief Frecuencia de muestreo del PPG en Hz. */

uint16_t valor_actual = 0;
static float ecg_filt[CHUNK] = {0};
static float ecg_muestra[CHUNK] = {0};

/*==================[internal data definition]===============================*/
/*! @brief Handle para la tarea de procesamiento y envío de datos del ECG. */
TaskHandle_t task_handle_ecg = NULL;

/*! @brief handle para la tarea de procesamiento y envío de datos del PPG.
 */
TaskHandle_t task_handle_ppg = NULL;

/*! @brief Configuración del canal ADC para leer el valor del PPG. */
analog_input_config_t poteInput = { 
    .input = CH2, 
};
/*! @brief Configuración del Bluetooth. */
ble_config_t ble_configuration = {
	"ESP_WAR_MACHINE",
	BLE_NO_INT };
/*==================[internal functions declaration]=========================*/
/** 
 * @brief Tarea que lee el valor del ECG, lo procesa y lo envía por Bluetooth.
 * 
*/
static void processAndSendEcg(void *pvParameter)
{
	uint16_t valor = 0;
	char msg[128];
	char msg_chunk[24];
	static uint8_t i = 0;
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		AnalogInputReadSingle(CH2, &valor);
		ecg_muestra[i] = valor;
		valor_actual = valor;
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
/**
 * @brief Tarea que lee el valor del PPG, lo procesa y lo envia por Bluetooth.
 * 
 * @param pvParameter Parametro de entrada (no utilizado).
 */
void procesAndSendPpg(void *pvParameter)
{	char msg[32];
    float analog_raw, analog_hp, analog_filt;
    uint16_t adcValue = 0;
    while(true){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        AnalogInputReadSingle(poteInput.input, &adcValue);
        analog_raw = (float)adcValue;

        HiPassFilter(&analog_raw, &analog_hp, 1);
        LowPassFilter(&analog_hp, &analog_filt, 1);
        snprintf(msg, sizeof(msg), "%0.2f\n", analog_filt);
		printf(">PPG:%0.2f\r\n", analog_filt);
    }
}
/** 
 * @brief Función asociada al temporizador del ECG.
 */
void FuncTimerSenialECG(void* param){
    xTaskNotifyGive(task_handle_ecg);
}
/**
 * @brief Función asociada al temporizador del PPG.
 */
void FuncTimerSenialPPG(void* param){
    xTaskNotifyGive(task_handle_ppg);
}

/**
 * @brief Inicializa los periféricos del sistema.
 * 
 * Configura los LEDs, filtros, Bluetooth, temporizadores, entradas analógicas,
 * salidas analógicas y UART.
 */
void inicialitePeripherals(){
	LedsInit();
	LedOn(LED_3); 
	LowPassInit(SAMPLE_FREQ, 20, ORDER_2);
    HiPassInit(SAMPLE_FREQ, 0.5, ORDER_2);

	BleInit(&ble_configuration);

	timer_config_t timer_ecg = {
	.timer = TIMER_B,
	.period = T_SENIAL,
	.func_p = FuncTimerSenialECG,
	.param_p = NULL
	};

	timer_config_t timer_ppg = {
	.timer = TIMER_A,
	.period = 4000,
	.func_p = FuncTimerSenialPPG,
	.param_p = NULL
	};

	TimerInit(&timer_ppg);
	TimerStart(timer_ppg.timer);
	TimerInit(&timer_ecg);
    TimerStart(timer_ecg.timer);

	AnalogInputInit(&poteInput);
	AnalogOutputInit();

	serial_config_t uart_config = {
		.baud_rate = 115200,
		.port = CH1,
		.func_p = NULL, 
		.param_p = NULL
   };
	UartInit(&uart_config);
	print("Configuracion de perifericos\n");

}
/*==================[external functions definition]==========================*/
void app_main(void){
	inicialitePeripherals();

	xTaskCreate(processAndSendEcg, "Leo y envio datos ECG", 2048, NULL, 5, &task_handle_ecg);
	xTaskCreate(procesAndSendPpg, "Leo y envio datos PPG", 2048, NULL, 5, &task_handle_ppg);

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
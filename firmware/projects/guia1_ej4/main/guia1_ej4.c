/*! @mainpage Guia1_ej4
 *
 * @section genDesc General Description
 * Consigna: Escriba una función que reciba un dato de 32 bits,  
 * la cantidad de dígitos de salida y un puntero a un arreglo 
 * donde se almacene los n dígitos. La función deberá convertir 
 * el dato recibido a BCD, guardando cada uno de los dígitos de 
 * salida en el arreglo pasado como puntero.
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
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
int8_t convertToBcdArray(uint32_t data, uint8_t n_digits, uint8_t *bcd_number){
	if (n_digits > 10){
		return -1;
	}
	for (int i = 0; i < n_digits; i++){
		bcd_number[i] = data % 10;
		data /= 10;
	}
	if (data >0){
		return -1; // el numero tiene mas digitos que los permitidos
	}
	return 0;
}


/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Hello world!\n");
	uint32_t numero = 12345;  // Número a convertir
    uint8_t digitos = 5;      // Cantidad de dígitos esperados
    uint8_t bcd[5];           // Arreglo para almacenar
    int8_t resultado = convertToBcdArray(numero, digitos, bcd);

    if (resultado == 0) {
        printf("BCD: ");
        for (int i = 0; i < digitos; i++) {
            printf("%d ", bcd[i]);
        }
        printf("\n");
    } else if (resultado == -1) {
        printf("Error: puntero nulo.\n");
    } else if (resultado == -2) {
        printf("Error: el número tiene más dígitos que los permitidos.\n");
    }

}
/*==================[end of file]============================================*/
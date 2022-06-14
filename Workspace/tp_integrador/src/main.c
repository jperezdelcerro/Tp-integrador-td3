#include "chip.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <string.h>

#define ON false
#define OFF true

#define PORT 0

/* CONTROL MATRIZ */
#define FILA_0 9
#define FILA_1 8
#define FILA_2 7
#define FILA_3 6

#define COLUMNA_0 0
#define COLUMNA_1 1
#define COLUMNA_2 18
#define COLUMNA_3 17

/* LEDS */
#define LED_SUCCESS 15
#define LED_FAILURE 16

/* TIME */
#define POOLING 5

char matriz[4][4] = {'1','2','3','A',
					 '4','5','6','B',
					 '7','8','9','C',
					 '*','0','#','D'};

typedef struct {
int x;
int y;
} coordinates;

char pruebarami[9];

void Configuracion(void){

	//####### GPIO #########

	Chip_GPIO_Init(LPC_GPIO);

	//####### LEDS #########

	Chip_GPIO_SetPinDIROutput(LPC_GPIO, PORT, LED_SUCCESS);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, PORT, LED_FAILURE);

	Chip_GPIO_SetPinState(LPC_GPIO, PORT, LED_SUCCESS, false);
	Chip_GPIO_SetPinState(LPC_GPIO, PORT, LED_FAILURE, false);

	//####### TECLADO #########

	Chip_GPIO_SetPinDIROutput(LPC_GPIO, PORT, FILA_0);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, PORT, FILA_1);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, PORT, FILA_2);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, PORT, FILA_3);

	Chip_GPIO_SetPinDIRInput(LPC_GPIO, PORT, COLUMNA_0);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, PORT, COLUMNA_1);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, PORT, COLUMNA_2);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, PORT, COLUMNA_3);

	Chip_GPIO_SetPinState(LPC_GPIO, PORT, FILA_0, OFF);
	Chip_GPIO_SetPinState(LPC_GPIO, PORT, FILA_1, OFF);
	Chip_GPIO_SetPinState(LPC_GPIO, PORT, FILA_2, OFF);
	Chip_GPIO_SetPinState(LPC_GPIO, PORT, FILA_3, OFF);

}



/* Tarea 1: Blinky del primer led */
static void Pooling(void *pvParameters){
	int length = 4;
	int filas[4] = {FILA_0, FILA_1, FILA_2, FILA_3};
	int columnas[4] = {COLUMNA_0, COLUMNA_1, COLUMNA_2, COLUMNA_3};
	int fila = 0;
	coordinates value;

	while(1) {
		Chip_GPIO_SetPinState(LPC_GPIO, PORT, filas[fila], ON);
		for(int columna = 0; columna < length; columna++) {


			int v = Chip_GPIO_GetPinState(LPC_GPIO, PORT, columnas[columna]);
			if (!v) {

				value.x = fila;
				value.y = columna;
				/*send myChar to colita*/
				char myChar = matriz[fila][columna];
				strncat(pruebarami, &myChar, 1);



				break;
			}
		}

		Chip_GPIO_SetPinState(LPC_GPIO, PORT, filas[fila], OFF);
		fila++;
		fila %= 4;
	}
}









/****************************************************************************************************/
/**************************************** MAIN ******************************************************/
/****************************************************************************************************/

int main(void){

	/* Levanta la frecuencia del micro */
	SystemCoreClockUpdate();

	/* Configuración inicial del micro */
	Configuracion();

    /* Creacion de tareas */
	xTaskCreate(Pooling, (char *) "Pooling",
    			configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
    			(xTaskHandle *) NULL);

    /* Inicia el scheduler */
	vTaskStartScheduler();

	/* Nunca debería arribar aquí */

    return 0;
}


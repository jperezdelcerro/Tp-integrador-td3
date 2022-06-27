#include "chip.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "lcd.h"
#include <string.h>
#include "debounce.h"
#include "memory.h"

#define ON false
#define OFF true

#define PORT 0
#define PORT1 1

/* CONTROL MATRIZ */
#define FILA_0 9
#define FILA_1 8
#define FILA_2 7
#define FILA_3 6

#define COLUMNA_0 0
#define COLUMNA_1 1
#define COLUMNA_2 18
#define COLUMNA_3 17

/*LCD*/
#define R 23
#define E 24
#define BIT_0 25
#define BIT_1 26
#define BIT_2 30
#define BIT_3 31

/* LEDS */
#define LED_SUCCESS 15
#define LED_FAILURE 16
#define led1_pin	22

/* TIME */
#define POLLING 50/portTICK_RATE_MS

char matriz[4][4] = {'1','2','3','A',
					 '4','5','6','B',
					 '7','8','9','C',
					 '*','0','#','D'};

/* POS CLAVE*/
#define LEN 5
#define ULTIMA 4

char claveInput[LEN];
char clave[LEN];

/* COLAS */
xQueueHandle queueValidation, queueLcd, queueMem;


/* FUNCIONES */

void strclean(char *str) {
	strncpy(str,"",strlen(str));
}

void writeToMemory(char text[LEN]) {
	char *value = text;
	writeOnFlash(value);
}


void readFromMemory(void){
	char *read = readMemory();
	for(int i = 0; i < LEN; i++) {
		clave[i] = *(read + i);
	}
}

void sendToLcd(char text[16]){
// Para no tener que guardar en text
	xQueueSendToBack(queueLcd, text, portMAX_DELAY);
}

void Configuracion(void){

	//####### GPIO #########
	Chip_GPIO_Init(LPC_GPIO);

	//####### LEDS #########

	Chip_GPIO_SetPinDIROutput(LPC_GPIO, PORT, LED_SUCCESS);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, PORT, LED_FAILURE);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, PORT, led1_pin);

	Chip_GPIO_SetPinState(LPC_GPIO, PORT, led1_pin, OFF);
	Chip_GPIO_SetPinState(LPC_GPIO, PORT, LED_SUCCESS, OFF);
	Chip_GPIO_SetPinState(LPC_GPIO, PORT, LED_FAILURE, OFF);

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

	//####### LCD ############

	LCD_SetUp(P0_15,P_NC,P0_16,P_NC,P_NC,P_NC,P_NC,P0_23,P0_24,P0_25,P0_26);
	LCD_Init(2,16);

	//####### MEMORIA ########

	readFromMemory();
}


/* TAREAS */

/* Tarea 1: Pooling de teclado y logica */
static void Polling(void *pvParameters){
	int length = 4;
	int filas[4] = {FILA_0, FILA_1, FILA_2, FILA_3};
	int columnas[4] = {COLUMNA_0, COLUMNA_1, COLUMNA_2, COLUMNA_3};
	int fila = 0;
	int index = 0;

	while(1) {
		Chip_GPIO_SetPinState(LPC_GPIO, PORT, filas[fila], ON);
		for(int columna = 0; columna < length; columna++) {
			int noPresionado = Chip_GPIO_GetPinState(LPC_GPIO, PORT, columnas[columna]);
			if (!noPresionado && debounce(fila, columna)) {

				char myChar = matriz[fila][columna];
				switch (myChar) {
					case 'A':
						if (strlen(claveInput) == ULTIMA) {
							if (strcmp(claveInput, clave) == 0) {
							        	sendToLcd("Clave Correcta");

							        }else{
							        	sendToLcd("ClaveIncorrecta");
							        }
						}else{
							sendToLcd("Clave muy corta");
						}

						strclean(claveInput);
						index=0;

						break;
					case 'B':

						sendToLcd("");
						strclean(claveInput);
						index = 0;

						break;
					case 'C':
						if (strlen(claveInput) == ULTIMA) {
							xQueueSendToBack(queueMem, &claveInput, portMAX_DELAY);
							sendToLcd("Clave guardada");
						}else{
							sendToLcd("Clave muy corta");
						}

						strclean(claveInput);
						index=0;


						break;
					default: ;
						if (index <  ULTIMA) {
							claveInput[index] = myChar;
							claveInput[index + 1] = '\0';
							index++;
							sendToLcd(claveInput);
							}

						if (index == LEN) {
							strclean(claveInput);
							index=0;
							}

						break;
				}
				break;


			} else {
				setHigh(fila, columna);
			}
		}
		vTaskDelay(POLLING);
		Chip_GPIO_SetPinState(LPC_GPIO, PORT, filas[fila], OFF);
		fila++;
		fila %= 4;
	}
}


/* Tarea 2: escritura en LCD */
static void Lcd(void *pvParameters) {
	LCD_Clear();
	char textToDisplay[16];
    while(1) {

    	strclean(textToDisplay);
    	xQueueReceive(queueLcd, &textToDisplay, portMAX_DELAY);
        LCD_Clear();
    	LCD_DisplayString(textToDisplay);
    }
}


/* Tarea 3: escritura en memoria y actualizacion de la vaiable global */
static void EscribirEnMemoria(void *pvParameters) {
    char nuevaClave[LEN];
	while(1) {
		strclean(nuevaClave);
		xQueueReceive(queueMem, &nuevaClave, portMAX_DELAY);
		writeToMemory(nuevaClave);
		strcpy(clave, nuevaClave); //actualizamos sin leer en tiempo de ejecucion
    }
}


int main(void){

	/* Levanta la frecuencia del micro */
	SystemCoreClockUpdate();

	/* Configuración inicial del micro */
	Configuracion();

	/* Creacion de colas */
	queueLcd = xQueueCreate(1, sizeof(char[16]));
	queueMem = xQueueCreate(1, sizeof(char[4]));

    /* Creacion de tareas */
	xTaskCreate(Polling, (char *) "Polling teclado y logica",
    			configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
    			(xTaskHandle *) NULL);

	xTaskCreate(Lcd, (char *) "Mostrar en LCD",
    			configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
    			(xTaskHandle *) NULL);

	xTaskCreate(EscribirEnMemoria, (char *) "Guardar nueva clave en memoria",
    			configMINIMAL_STACK_SIZE, NULL, ((tskIDLE_PRIORITY + 1UL)| portPRIVILEGE_BIT),
    			(xTaskHandle *) NULL);

    /* Inicia el scheduler */
	vTaskStartScheduler();

	/* Nunca debería arribar aquí */

    return 0;
}

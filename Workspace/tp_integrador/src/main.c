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

xQueueHandle queueValidation, queueLcd, queueMem;

char claveInput[4];
char *textoDisplay = claveInput;
char clave[4];

void strclean(char *str) {
	for(int i = 3; i > 0; i--) {
		str[i] = '\0';
	}
}

void writeFcknMem(char text[4]) {
	char *value = text;
	int l = strlen(text);
	writeOnFlash(value);
//	char *read = readMemory();
//	for(int i = 0; i < 4; i++) {
//		clave[i] = *(read + i);
//	}
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
//	char *value = "1111";
//	writeOnFlash(value);
//	writeFcknMem("1111");
	char *read = readMemory();
	for(int i = 0; i < 4; i++) {
		clave[i] = *(read + i);
	}
}

/* Tarea 1: Blinky del primer led */
static void Polling(void *pvParameters){
	int length = 4;
	int filas[4] = {FILA_0, FILA_1, FILA_2, FILA_3};
	int columnas[4] = {COLUMNA_0, COLUMNA_1, COLUMNA_2, COLUMNA_3};
	int fila = 0;
	int index = 0;
	bool setearClave = false;

	while(1) {
		Chip_GPIO_SetPinState(LPC_GPIO, PORT, filas[fila], ON);
		for(int columna = 0; columna < length; columna++) {
			int noPresionado = Chip_GPIO_GetPinState(LPC_GPIO, PORT, columnas[columna]);
			if (!noPresionado && debounce(fila, columna)) {
				char myChar = matriz[fila][columna];
				switch (myChar) {
					case 'A':
						if (!setearClave) {
							xQueueSendToBack(queueValidation, &claveInput, portMAX_DELAY);
							strclean(claveInput);
							index = 0;
						}
						break;
					case 'B':
						if (!setearClave) {
							strclean(claveInput);
							index = 0;
						}
						break;
					case 'C':
						textoDisplay = clave[0];
						setearClave = true;
						strclean(clave);
						index = 0;
						break;
					case 'D':
						if (setearClave && strlen(clave) == 4) {
							xQueueSendToBack(queueMem, &clave, portMAX_DELAY);
							textoDisplay = claveInput[0];
							setearClave = false;
							index = 0;
						}
						break;
					default: ;
						char *escribirEn = setearClave ? clave : claveInput;
						if (strlen(escribirEn) <= 4) {
							escribirEn[index] = myChar;
							index++;
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

static void Validation(void *pvParameters) {
    char claveIngresada[5];
    int isValid = 1;
    while(1) {
        xQueueReceive(queueValidation, &claveIngresada, portMAX_DELAY);
        claveIngresada[4] = '\0';
        isValid = strcmp(claveIngresada, clave) == 0;
    	xQueueSendToBack(queueLcd, &isValid, portMAX_DELAY);
        if (isValid) {
        	//clave correcta!!
        	for(int i = 0; i < 3; i++) {
				Chip_GPIO_SetPinState(LPC_GPIO, PORT, led1_pin, ON);
				vTaskDelay(200/portTICK_RATE_MS);
				Chip_GPIO_SetPinState(LPC_GPIO, PORT, led1_pin, OFF);
				vTaskDelay(200/portTICK_RATE_MS);
        	}
        }
    }
}


static void Lcd(void *pvParameters) {
	int isValid;
	LCD_Clear();
	char display[4];
    while(1) {
    	if(xQueueReceive(queueLcd, &isValid, 200/portTICK_RATE_MS)){
    		if(isValid){
        		LCD_DisplayString("Clave Correcta");
    		}else{
    			LCD_DisplayString("Clave Incorrecta");
    		}
    		vTaskDelay(2000/portTICK_RATE_MS);
        	LCD_Clear();
    	}else{
        	LCD_DisplayString("Hola!");
    		vTaskDelay(1000/portTICK_RATE_MS);
        	LCD_Clear();
    	}
    }
}

static void writeToMem(void *pvParameters) {
    char value[4], *read;
	while(1) {
		xQueueReceive(queueMem, &value, portMAX_DELAY);
		value[4] = '\0';
		writeFcknMem(value);
    }
}


int main(void){

	/* Levanta la frecuencia del micro */
	SystemCoreClockUpdate();

	/* Configuración inicial del micro */
	Configuracion();

	queueValidation = xQueueCreate(1, sizeof(char[4]));
	queueLcd = xQueueCreate(1, sizeof(int));
	queueMem = xQueueCreate(1, sizeof(char[4]));

    /* Creacion de tareas */
	xTaskCreate(Polling, (char *) "Polling",
    			configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
    			(xTaskHandle *) NULL);

    xTaskCreate(Validation, (char *) "Validacion",
    			configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
    			(xTaskHandle *) NULL);

	xTaskCreate(Lcd, (char *) "lcd",
    			configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
    			(xTaskHandle *) NULL);

	xTaskCreate(writeToMem, (char *) "Guardar",
    			configMINIMAL_STACK_SIZE, NULL, ((tskIDLE_PRIORITY + 1UL)| portPRIVILEGE_BIT),
    			(xTaskHandle *) NULL);

    /* Inicia el scheduler */
	vTaskStartScheduler();

	/* Nunca debería arribar aquí */

    return 0;
}

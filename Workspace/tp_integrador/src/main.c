#include "chip.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <string.h>
#include "debounce.h"
#include "lcd.h"


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

xQueueHandle queue;
xQueueHandle queueLcd;

char claveInput[5];
char *textoDisplay = claveInput;
char clave[5] = "1111\0";

void strclean(char *str) {
	for(int i = 0; i < 4; i++) {
		str[i] = '\0';
	}
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

	LCD_SetUp(P0_23,P_NC,P0_24,P_NC,P_NC,P_NC,P_NC,P0_25,P0_26,P1_30,P1_31);
	LCD_Init(2,16);

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
							xQueueSendToBack(queue, &claveInput, portMAX_DELAY);
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
						textoDisplay = clave;
						setearClave = true;
						strclean(clave);
						index = 0;
						break;
					case 'D':
						if (setearClave && strlen(clave) == 4) {
							textoDisplay = claveInput;
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
        xQueueReceive(queue, &claveIngresada, portMAX_DELAY);
     //   claveIngresada[4] = '\0';
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
        	LCD_DisplayString(textoDisplay);
    		vTaskDelay(1000/portTICK_RATE_MS);
        	LCD_Clear();
    	}
    }
}

static void Add(void *pvParameters) {
	int isValid;
    while(1) {
    	for(int i=0; i<5;i++){
    		claveInput[i] = '1';
    		vTaskDelay(1500/portTICK_RATE_MS);
    	}
    	xQueueSendToBack(queue, &claveInput, portMAX_DELAY);
    	strclean(claveInput);
    }
}


int main(void){

	/* Levanta la frecuencia del micro */
	SystemCoreClockUpdate();

	/* Configuración inicial del micro */
	Configuracion();
	queue = xQueueCreate(1, sizeof(char[4]));
	queueLcd = xQueueCreate(1, sizeof(int));

    /* Creacion de tareas */
	xTaskCreate(Polling, (char *) "Polling",
    			configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
    			(xTaskHandle *) NULL);

    /* Creacion de tareas */
	xTaskCreate(Validation, (char *) "Validacion",
    			configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
    			(xTaskHandle *) NULL);

    /* Creacion de tareas */
	xTaskCreate(Lcd, (char *) "lcd",
    			configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
    			(xTaskHandle *) NULL);

    /* Creacion de tareas */
	xTaskCreate(Add, (char *) "add",
    			configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
    			(xTaskHandle *) NULL);

    /* Inicia el scheduler */
	vTaskStartScheduler();

	/* Nunca debería arribar aquí */

    return 0;
}


#include "memory.h"
#include "string.h"

bool writeOnFlash(char dataToWrite[4]){
	uint8_t * puntero;
	char aux[4];
	char value[256];
	strcpy(aux, dataToWrite);
	//1) Generamos un string a escribir. DEBE estar en RAM.
	sprintf(value,aux);
	//2) Apuntamos a la zona de memoria con un puntero
	puntero=value;

	//3) Llamamos a Grabar Flash. Esta funcion es muy muy basica y graba siempre en el sector 16
	if(grabar_flash_sector_16(puntero,256))
		return true;
	return false;
}

char* readMemory(){
	char * punteroFLASH, variableFLASH[4];
	punteroFLASH = IMG_START_SECTOR;
	return punteroFLASH;
}

bool grabar_flash_sector_16(uint8_t *data, uint16_t length){
	uint32_t valor;
//	SystemCoreClockUpdate();
	/*	Erase the images stored in flash */
	if ((Chip_IAP_PreSectorForReadWrite(16, 16) == IAP_CMD_SUCCESS) &&(Chip_IAP_EraseSector(16, 16) == IAP_CMD_SUCCESS)){
		if(length > 0){
			/*	Prepare Sectors to be flashed */
			if(Chip_IAP_PreSectorForReadWrite(16, 16) == IAP_CMD_SUCCESS){
				/*	Copy data (already) located in RAM to flash */
				if( (valor=Chip_IAP_CopyRamToFlash(IMG_START_SECTOR, (uint32_t)data, length)) == IAP_CMD_SUCCESS){
					/*	Verify the flash contents with the contents in RAM */
					if(Chip_IAP_Compare(IMG_START_SECTOR, (uint32_t)data, length) == IAP_CMD_SUCCESS){
						return true;
					}
				}
			}
			return false;
		}else{
			//Si llega aqui, la longitud era 0
			return true;
		}

	}else{
		return false;
	}
}

bool readAndCheck(uint32_t *dataToCheck){

	uint8_t * punteroFLASH, variableFLASH[4], variableUSER[4];

	punteroFLASH=IMG_START_SECTOR;

	for (uint16_t i=0; i<4; i++){
		variableFLASH[i]=*(punteroFLASH+i);
		variableUSER[i]=(*(dataToCheck+i)+48);
	}
	if(variableFLASH[0]==variableUSER[0]&&variableFLASH[1]==variableUSER[1]&&variableFLASH[2]==variableUSER[2]&&variableFLASH[3]==variableUSER[3])
		return true;
	return false;

}

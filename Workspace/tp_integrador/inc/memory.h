#define	IMG_START_SECTOR 0x00010000
#define HEADER_SIZE 8
#include "FreeRTOS.h"
#include "iap.h"
#include <stdbool.h>

bool grabar_flash_sector_16(uint8_t *data, uint16_t length);
bool writeOnFlash(char *dataToWrite);
char* readMemory();

#include "debounce.h"
#include "string.h"

/*Si el estado nuevo es distinto del anterior
 * devuelvo true, esto evita que haya dos estados altos
 * seguidos sin pasar por estado bajo.
 */
int debounce(int fila, int columna) {
	int prev[4][4];
    memcpy(prev, buttonStates, FILAS*COLUMNAS*sizeof(int));
	buttonStates[fila][columna] = LOW;
	return prev[fila][columna] != buttonStates[fila][columna];
}

void setHigh(int fila, int columna) {
	buttonStates[fila][columna] = ALTO;
}

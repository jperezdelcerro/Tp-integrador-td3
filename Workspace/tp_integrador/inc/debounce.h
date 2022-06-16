#define ALTO 1
#define LOW 0
#define FILAS 4
#define COLUMNAS 4

static int buttonStates[FILAS][COLUMNAS] = {
		ALTO, ALTO, ALTO, ALTO,
		ALTO, ALTO, ALTO, ALTO,
		ALTO, ALTO, ALTO, ALTO,
		ALTO, ALTO, ALTO, ALTO
	};

int debounce(int fila, int columna);

void setHigh(int fila, int columna);

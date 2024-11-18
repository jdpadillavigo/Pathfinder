#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <unordered_map>
#include "raylib.h"

extern Font fuentePersonalizada;

// Constantes para representar el estado de las celdas
const int VACIO = 0;
const int BLOQUEADO = 1;
const int ORIGEN = 2;
const int DESTINO = 3;
const int RUTA = 4;
const int CELDA_TAM = 40;
const int INSTRUCCIONES_ALTURA = 240;  // Altura de la sección de instrucciones

// Definición de la estructura Celda
struct Celda {
    int fila;
    int columna;
    int estado;
    bool visitado;
    Celda* padre;
    float g, h;  // g: coste acumulado, h: heurística
    float f;     // f = g + h (coste total)

    Celda();
    Celda(int f, int c);
    bool operator>(const Celda& otro) const;
};

// Definición de la clase Grid
class Grid {
private:
    std::vector<std::vector<Celda>> celdas;
    int filas;
    int columnas;

public:
    Grid(int f, int c);

    Celda& obtenerCelda(int fila, int columna);
    void dibujarGrilla() const;
    void reiniciar(bool reiniciarBloqueados = false, bool reiniciarOrigenDestino = false);

    bool BFS(Celda& origen, Celda& destino);
    bool DStarLite(Celda& origen, Celda& destino);
    float calcularHeuristica(const Celda& origen, const Celda& destino) const;
};

void dibujarInstrucciones(bool modoEdicion);

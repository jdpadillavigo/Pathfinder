#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <unordered_map>
#include "raylib.h" // Biblioteca para la interfaz gráfica (GUI)

// Fuente personalizada para dibujar texto
extern Font fuentePersonalizada;

// Constantes para representar los diferentes estados de las celdas en la grilla
const int VACIO = 0;
const int BLOQUEADO = 1;
const int ORIGEN = 2;
const int DESTINO = 3;
const int RUTA = 4;
const int CELDA_TAM = 40; // Tamaño de cada celda en píxeles
const int INSTRUCCIONES_ALTURA = 240; // Altura de la sección de instrucciones en la interfaz

// Estructura que representa una celda en la grilla
struct Celda {
    int fila;
    int columna;
    int estado; // Estado de la celda: vacío, bloqueado, origen, destino, ruta
    bool visitado; // Indica si la celda ha sido visitada
    Celda* padre; // Puntero al padre en el camino
    float g, h;   // g: coste acumulado, h: heurística
    float f;      // f = g + h (coste total)

    // Constructor por defecto
    Celda();

    // Constructor con fila y columna
    Celda(int f, int c);

    // Operador de comparación para usar en la cola de prioridad (min-heap)
    bool operator>(const Celda& otro) const;
};

// Clase que representa la grilla de celdas
class Grid {
private:
    std::vector<std::vector<Celda>> celdas; // Matriz de celdas
    int filas;
    int columnas;

public:
    // Constructor que crea una grilla de celdas
    Grid(int f, int c);

    // Método para obtener una celda en una posición específica
    Celda& obtenerCelda(int fila, int columna);

    // Método para dibujar la grilla de celdas en la pantalla
    void dibujarGrilla() const;

    // Método para reiniciar la grilla, puede reiniciar celdas bloqueadas y las celdas de origen/destino
    void reiniciar(bool reiniciarBloqueados = false, bool reiniciarOrigenDestino = false);

    // Método para reiniciar las estructuras específicas del algoritmo D* Lite
    void reiniciarAlgoritmoDStarLite();

    // Función de búsqueda BFS
    bool BFS(Celda& origen, Celda& destino);

    // Función de búsqueda D* Lite
    bool DStarLite(Celda& origen, Celda& destino);

    // Función de cálculo de heurística (distancia de Manhattan)
    float calcularHeuristica(const Celda& origen, const Celda& destino) const;
};

// Función que dibuja las instrucciones en pantalla
void dibujarInstrucciones(bool modoEdicion);

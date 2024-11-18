#include "entidades.h"

Font fuentePersonalizada;

// Constructor por defecto de Celda
Celda::Celda() : fila(-1), columna(-1), estado(VACIO), visitado(false), padre(nullptr), g(INFINITY), h(0), f(INFINITY) {}

// Constructor con parámetros de fila y columna
Celda::Celda(int f, int c) : fila(f), columna(c), estado(VACIO), visitado(false), padre(nullptr), g(INFINITY), h(0), f(INFINITY) {}

// Operador de comparación para la cola de prioridad (min-heap)
bool Celda::operator>(const Celda& otro) const {
    return f > otro.f;
}

// Constructor de la clase Grid
Grid::Grid(int f, int c) : filas(f), columnas(c) {
    celdas.resize(filas, std::vector<Celda>(columnas));
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            celdas[i][j] = Celda(i, j);
        }
    }
}

// Obtener una celda específica
Celda& Grid::obtenerCelda(int fila, int columna) {
    return celdas[fila][columna];
}

// Dibujar la grilla en pantalla
void Grid::dibujarGrilla() const {
    Color celesteClaro = Color{184, 237, 255, 255};
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            Color color;
            switch (celdas[i][j].estado) {
                case VACIO: color = celesteClaro; break;
                case BLOQUEADO: color = RED; break;
                case ORIGEN: color = GREEN; break;
                case DESTINO: color = YELLOW; break;
                case RUTA: color = BLUE; break;
                default: color = celesteClaro; break;
            }
            Vector2 posicion = { (float)(j * CELDA_TAM), (float)(i * CELDA_TAM + INSTRUCCIONES_ALTURA) };  // Ajustamos la posición en Y
            DrawRectangleV(posicion, {CELDA_TAM, CELDA_TAM}, color);
            // Dibujamos el borde negro de cada celda
            DrawRectangleLinesEx({posicion.x, posicion.y, (float)CELDA_TAM, (float)CELDA_TAM}, 0.5, BLACK);
            if (celdas[i][j].estado == ORIGEN || celdas[i][j].estado == DESTINO) {
                const char* texto = (celdas[i][j].estado == ORIGEN) ? "O" : "D";
                Vector2 size = MeasureTextEx(fuentePersonalizada, texto, 20, 0);
                float posX = posicion.x + (CELDA_TAM - size.x) / 2;  // Centrado horizontal
                float posY = posicion.y + (CELDA_TAM - size.y) / 2;  // Centrado vertical
                DrawTextEx(fuentePersonalizada, texto, { posX, posY }, 20, 0, BLACK);
            }
        }
    }
}

// Reiniciar la grilla, con opciones para reiniciar bloqueos y origen/destino
void Grid::reiniciar(bool reiniciarBloqueados, bool reiniciarOrigenDestino) {
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            Celda& celda = celdas[i][j];
            // Si reiniciarOrigenDestino es true, restablecemos el origen y destino a vacío
            if (reiniciarOrigenDestino && (celda.estado == ORIGEN || celda.estado == DESTINO)) {
                celda.estado = VACIO;  // Ponemos las celdas de origen y destino como vacías
            }
            // No reiniciamos las celdas de ORIGEN ni DESTINO si no se indica
            if (celda.estado == ORIGEN || celda.estado == DESTINO) {
                continue;  // Saltamos la modificación de origen y destino
            }
            // Solo restablecemos celdas que no están bloqueadas si no se pide reiniciar bloqueados
            if (celda.estado != BLOQUEADO || reiniciarBloqueados) {
                celda.estado = VACIO;  // Restablecemos el estado a vacío
            }
            celda.visitado = false;
            celda.padre = nullptr;
            celda.g = INFINITY;
            celda.h = 0;
            celda.f = INFINITY;
        }
    }
}

// Algoritmo BFS (Breadth-First Search)
bool Grid::BFS(Celda& origen, Celda& destino) {
    reiniciar();
    std::queue<Celda*> cola;
    origen.visitado = true;
    cola.push(&origen);

    while (!cola.empty()) {
        Celda* actual = cola.front();
        cola.pop();

        if (actual == &destino) {
            break;
        }

        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (i == 0 || j == 0) {  // Aseguramos que solo sean direcciones cardinales
                    if (i == 0 && j == 0) continue;
                    int nuevaFila = actual->fila + i;
                    int nuevaColumna = actual->columna + j;
                    if (i != 0 && j != 0) continue;
                    if (nuevaFila >= 0 && nuevaFila < filas && nuevaColumna >= 0 && nuevaColumna < columnas) {
                        Celda& vecino = obtenerCelda(nuevaFila, nuevaColumna);
                        if (!vecino.visitado && vecino.estado != BLOQUEADO) {
                            vecino.visitado = true;
                            vecino.padre = actual;
                            cola.push(&vecino);
                        }
                    }
                }
            }
        }
    }

    if (destino.padre != nullptr) {
        Celda* temp = &destino;
        while (temp != nullptr) {
            if (temp != &origen && temp != &destino) {
                temp->estado = RUTA;
            }
            temp = temp->padre;
        }
        return true;
    }
    return false;
}

// Algoritmo D* Lite
bool Grid::DStarLite(Celda& origen, Celda& destino) {
    reiniciar();
    std::unordered_map<Celda*, float> colaPrioridad;
    origen.g = 0;
    origen.h = calcularHeuristica(origen, destino);
    origen.f = origen.g + origen.h;
    colaPrioridad[&origen] = origen.f;

    while (!colaPrioridad.empty()) {
        Celda* actual = nullptr;
        float menorF = INFINITY;
        for (auto& entry : colaPrioridad) {
            if (entry.second < menorF) {
                menorF = entry.second;
                actual = entry.first;
            }
        }
        colaPrioridad.erase(actual);

        if (actual == &destino) {
            // Hemos llegado al destino, reconstruimos la ruta
            Celda* temp = actual;
            while (temp != nullptr) {
                // Asegurarnos de no cambiar el origen ni el destino a "RUTA"
                if (temp != &origen && temp != &destino) {
                    temp->estado = RUTA;
                }
                temp = temp->padre;
            }
            return true;
        }

        // Exploramos las 4 celdas vecinas (arriba, abajo, izquierda, derecha)
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (i == 0 && j == 0) continue; // No procesamos la celda actual
                int nuevaFila = actual->fila + i;
                int nuevaColumna = actual->columna + j;

                if (nuevaFila >= 0 && nuevaFila < filas && nuevaColumna >= 0 && nuevaColumna < columnas) {
                    Celda& vecino = obtenerCelda(nuevaFila, nuevaColumna);
                    
                    // Si la celda no está bloqueada y no ha sido visitada, la procesamos
                    if (vecino.estado != BLOQUEADO) {
                        float nuevoG = actual->g + 1;  // Suponemos que el coste de movernos entre celdas es 1
                        if (nuevoG < vecino.g) {
                            vecino.g = nuevoG;
                            vecino.h = calcularHeuristica(vecino, destino);
                            vecino.f = vecino.g + vecino.h;
                            vecino.padre = actual;
                            colaPrioridad[&vecino] = vecino.f;  // Actualizamos la celda en la cola de prioridad
                        }
                    }
                }
            }
        }
    }

    return false;
}

// Función para calcular la heurística (distancia Euclidiana)
float Grid::calcularHeuristica(const Celda& origen, const Celda& destino) const {
    return abs(origen.fila - destino.fila) + abs(origen.columna - destino.columna);
}

// Función para dibujar las instrucciones en pantalla
void dibujarInstrucciones(bool modoEdicion) {
    const char* instrucciones = 
        "INSTRUCCIONES:\n\n"
        "1. Presiona CTRL para activar/desactivar el modo edicion.\n"
        "2. En modo edicion, Clic Izquierdo bloquea casillas (rojo).\n"
        "3. En modo edicion, Clic Derecho desbloquea casillas (celeste claro).\n"
        "4. Presiona Clic Izquierdo para seleccionar origen (verde) y destino (amarillo).\n"
        "5. Presiona ENTER para buscar ruta (BFS) entre origen y destino.\n"
        "6. Presiona ESPACIO para buscar ruta (D* Lite) entre origen y destino.\n"
        "7. Presiona ESC para reiniciar la grilla a su estado inicial.";

    // Fondo de las instrucciones
    DrawRectangle(0, 0, GetScreenWidth(), INSTRUCCIONES_ALTURA, Color{30, 42, 71, 255});  // Azul oscuro

    // Centrar "Buscador de Rutas" en la parte superior de la pantalla
    const char* titulo = "Buscador de Rutas";
    int anchoTexto = MeasureText(titulo, 30);  // Medir el ancho del texto
    int posicionX = (GetScreenWidth() - anchoTexto) / 2;  // Centrar horizontalmente
    DrawTextEx(fuentePersonalizada, titulo, { (float)posicionX, 20 }, 30, 0, Color{240, 240, 240, 255});  // Blanco para el título "Buscador de Rutas"

    int posicionInstruccionesY = 60;  // Distancia entre las instrucciones y el título
    DrawTextEx(fuentePersonalizada, instrucciones, {10.0f, (float)posicionInstruccionesY}, 20, 0, Color{190, 190, 190, 255});  // Gris claro para instrucciones

    // Si estamos en modo edición, mostrar un aviso adicional
    if (modoEdicion) {
        const char* mensajeModoEdicion = "Modo Edicion Activado! Presiona CTRL para desactivar";
        int anchoMensaje = MeasureText(mensajeModoEdicion, 20);  // Medir el ancho del mensaje
        int posicionXMensaje = (GetScreenWidth() - anchoMensaje) / 2;  // Centrar horizontalmente
        DrawTextEx(fuentePersonalizada, mensajeModoEdicion, { (float)posicionXMensaje, (float)(INSTRUCCIONES_ALTURA - 30) }, 20, 0, Color{255, 0, 0, 255});  // Rojo brillante para el aviso
    }
}

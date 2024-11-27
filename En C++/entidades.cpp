#include "entidades.h"

// Fuente personalizada para dibujar texto
Font fuentePersonalizada;

// Constructor por defecto
Celda::Celda() : fila(-1), columna(-1), estado(VACIO), visitado(false), padre(nullptr), g(INFINITY), h(0), f(INFINITY) {}

// Constructor con fila y columna
Celda::Celda(int f, int c) : fila(f), columna(c), estado(VACIO), visitado(false), padre(nullptr), g(INFINITY), h(0), f(INFINITY) {}

// Operador de comparación para usar en la cola de prioridad (min-heap)
bool Celda::operator>(const Celda& otro) const {
    return f > otro.f; // La celda con mayor f (coste total) se considera "mayor"
}

// Constructor que crea una grilla de celdas
Grid::Grid(int f, int c) : filas(f), columnas(c) {
    celdas.resize(filas, std::vector<Celda>(columnas)); // Redimensionar la grilla
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            celdas[i][j] = Celda(i, j); // Inicializar cada celda con sus coordenadas
        }
    }
}

// Método para obtener una celda en una posición específica
Celda& Grid::obtenerCelda(int fila, int columna) {
    return celdas[fila][columna];
}

// Método para dibujar la grilla de celdas en la pantalla
void Grid::dibujarGrilla() const {
    Color celesteClaro = Color{184, 237, 255, 255}; // Color de las celdas vacías
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            Color color; // Variable para almacenar el color de la celda
            // Asignar un color dependiendo del estado de la celda
            switch (celdas[i][j].estado) {
                case VACIO: color = celesteClaro; break;
                case BLOQUEADO: color = RED; break;
                case ORIGEN: color = GREEN; break;
                case DESTINO: color = YELLOW; break;
                case RUTA: color = BLUE; break;
                default: color = celesteClaro; break;
            }
            // Posición de la celda en la pantalla
            Vector2 posicion = { (float)(j * CELDA_TAM), (float)(i * CELDA_TAM + INSTRUCCIONES_ALTURA) };
            // Dibujar el rectángulo representando la celda
            DrawRectangleV(posicion, {CELDA_TAM, CELDA_TAM}, color);
            // Dibujar el borde de la celda en color negro
            DrawRectangleLinesEx({posicion.x, posicion.y, (float)CELDA_TAM, (float)CELDA_TAM}, 0.5, BLACK);

            // Si la celda es origen o destino, dibujamos el texto "O" o "D"
            if (celdas[i][j].estado == ORIGEN || celdas[i][j].estado == DESTINO) {
                const char* texto = (celdas[i][j].estado == ORIGEN) ? "O" : "D"; // Determinamos qué texto mostrar
                Vector2 size = MeasureTextEx(fuentePersonalizada, texto, 20, 0); // Medimos el tamaño del texto

                // Centramos el texto en la celda
                float posX = posicion.x + (CELDA_TAM - size.x) / 2;  // Centrado horizontal
                float posY = posicion.y + (CELDA_TAM - size.y) / 2;  // Centrado vertical

                // Dibujamos el texto en la celda
                DrawTextEx(fuentePersonalizada, texto, { posX, posY }, 20, 0, BLACK);
            }
        }
    }
}

// Método para reiniciar la grilla, puede reiniciar celdas bloqueadas y las celdas de origen/destino
void Grid::reiniciar(bool reiniciarBloqueados, bool reiniciarOrigenDestino) {
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            Celda& celda = celdas[i][j];
            
            // Si se indica, reiniciamos las celdas de origen y destino
            if (reiniciarOrigenDestino && (celda.estado == ORIGEN || celda.estado == DESTINO)) {
                celda.estado = VACIO;  // Ponemos origen y destino como celdas vacías
            }

            // No reiniciamos las celdas de ORIGEN ni DESTINO si no se indica
            if (celda.estado == ORIGEN || celda.estado == DESTINO) {
                continue;
            }

            // Reiniciamos las celdas si no están bloqueadas o si se pide reiniciar bloqueadas
            if (celda.estado != BLOQUEADO || reiniciarBloqueados) {
                celda.estado = VACIO;  // Restablecemos el estado a vacío
            }

            // Reiniciamos los valores de búsqueda
            celda.visitado = false; // Marcamos la celda como no visitada
            celda.padre = nullptr;  // Restablecemos el padre
            celda.g = INFINITY;     // Coste acumulado infinito
            celda.h = 0;            // Heurística a 0
            celda.f = INFINITY;     // Coste total infinito
        }
    }
}

// Método para reiniciar las estructuras específicas del algoritmo D* Lite
void Grid::reiniciarAlgoritmoDStarLite() {
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            Celda& celda = obtenerCelda(i, j);
            celda.g = INFINITY;  // Reiniciar el costo acumulado
            celda.f = INFINITY;  // Reiniciar el costo de la solución
            celda.padre = nullptr;  // Reiniciar el predecesor
        }
    }
}

// Función de búsqueda BFS
bool Grid::BFS(Celda& origen, Celda& destino) {
    reiniciar(); // Restablecer el estado de todas las celdas antes de comenzar la búsqueda

    std::queue<Celda*> cola; // Cola para explorar las celdas en orden de BFS
    origen.visitado = true; // Marcar la celda origen como visitada
    cola.push(&origen); // Añadir la celda origen a la cola para empezar la exploración

    // Mientras haya celdas por explorar en la cola
    while (!cola.empty()) {
        Celda* actual = cola.front(); // Obtener la celda actual (primera en la cola)
        cola.pop(); // Eliminar la celda de la cola

        // Si hemos llegado al destino, terminamos la búsqueda
        if (actual == &destino) {
            break;
        }

        // Revisamos las 4 celdas vecinas (arriba, abajo, izquierda, derecha)
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                // Aseguramos que solo exploramos las 4 direcciones cardinales (no diagonales)
                if (i == 0 || j == 0) {  // Solo direcciones cardinales
                    if (i == 0 && j == 0) continue; // No procesamos la celda actual

                    // Calculamos las nuevas posiciones de fila y columna
                    int nuevaFila = actual->fila + i;
                    int nuevaColumna = actual->columna + j;
                    if (i != 0 && j != 0) continue; // Aseguramos que no procesamos las diagonales

                    // Verificamos si la celda vecina está dentro de los límites y no está bloqueada
                    if (nuevaFila >= 0 && nuevaFila < filas && nuevaColumna >= 0 && nuevaColumna < columnas) {
                        Celda& vecino = obtenerCelda(nuevaFila, nuevaColumna);
                        if (!vecino.visitado && vecino.estado != BLOQUEADO) {
                            vecino.visitado = true; // Marcamos la celda vecina como visitada
                            vecino.padre = actual;  // Guardamos al padre para reconstruir la ruta
                            cola.push(&vecino);     // Añadimos la celda vecina a la cola
                        }
                    }
                }
            }
        }
    }

    // Si hemos encontrado el destino (su padre no es nullptr), reconstruimos la ruta
    if (destino.padre != nullptr) {
        Celda* temp = &destino;
        while (temp != nullptr) {
            // Aseguramos que no cambiamos el origen ni el destino a "RUTA"
            if (temp != &origen && temp != &destino) {
                temp->estado = RUTA; // Marcamos la celda como parte de la ruta
            }
            temp = temp->padre; // Retrocedemos hacia el origen
        }
        return true; // Ruta encontrada
    }
    return false; // No se encontró una ruta
}

// Función de búsqueda D* Lite
bool Grid::DStarLite(Celda& origen, Celda& destino) {
    reiniciar(); // Restablecer el estado de todas las celdas

    // Cola de prioridad que guarda celdas con su coste f (g + h)
    std::unordered_map<Celda*, float> colaPrioridad;

    // Inicializamos la celda origen con costes g, h, y f
    origen.g = 0;
    origen.h = calcularHeuristica(origen, destino); // Calculamos la heurística
    origen.f = origen.g + origen.h;
    colaPrioridad[&origen] = origen.f; // Añadimos la celda origen a la cola de prioridad

    // Mientras haya celdas en la cola de prioridad
    while (!colaPrioridad.empty()) {
        // Buscar la celda con el menor coste f
        Celda* actual = nullptr;
        float menorF = INFINITY;
        for (auto& entry : colaPrioridad) {
            if (entry.second < menorF) {
                menorF = entry.second;
                actual = entry.first;
            }
        }

        colaPrioridad.erase(actual); // Eliminamos la celda con el menor coste f

        // Si hemos llegado al destino, reconstruimos la ruta
        if (actual == &destino) {
            // Hemos llegado al destino, reconstruimos la ruta
            Celda* temp = actual;
            while (temp != nullptr) {
                // Asegurarnos de no cambiar el origen ni el destino a "RUTA"
                if (temp != &origen && temp != &destino) {
                    temp->estado = RUTA; // Marcamos la celda como parte de la ruta
                }
                temp = temp->padre; // Retrocedemos hacia el origen
            }
            return true; // Ruta encontrada
        }

        // Exploramos las 4 celdas vecinas (arriba, abajo, izquierda, derecha)
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (i == 0 && j == 0) continue; // No procesamos la celda actual

                // Solo exploramos celdas ortogonales (sin diagonales)
                if (i != 0 && j != 0) continue;

                int nuevaFila = actual->fila + i;
                int nuevaColumna = actual->columna + j;

                // Verificamos que la celda vecina esté dentro de los límites
                if (nuevaFila >= 0 && nuevaFila < filas && nuevaColumna >= 0 && nuevaColumna < columnas) {
                    Celda& vecino = obtenerCelda(nuevaFila, nuevaColumna);
                    
                    // Si la celda no está bloqueada, la procesamos
                    if (vecino.estado != BLOQUEADO) {
                        float nuevoG = actual->g + 1;  // Suponemos que el coste de movernos entre celdas es 1
                        if (nuevoG < vecino.g) { // Si encontramos un camino más corto
                            vecino.g = nuevoG; // Actualizamos el coste g
                            vecino.h = calcularHeuristica(vecino, destino); // Calculamos la heurística
                            vecino.f = vecino.g + vecino.h; // Actualizamos el coste total f
                            vecino.padre = actual; // Guardamos el padre para reconstruir la ruta
                            colaPrioridad[&vecino] = vecino.f;  // Actualizamos la celda en la cola de prioridad
                        }
                    }
                }
            }
        }
    }

    return false; // No se encontró una ruta
}

// Función de cálculo de heurística (distancia de Manhattan)
float Grid::calcularHeuristica(const Celda& origen, const Celda& destino) const {
    // Heurística de Manhattan (distancia en línea recta)
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

    // Fondo de las instrucciones (color azul oscuro)
    DrawRectangle(0, 0, GetScreenWidth(), INSTRUCCIONES_ALTURA, Color{30, 42, 71, 255});

    // Centrar "Buscador de Rutas" en la parte superior de la pantalla
    const char* titulo = "Buscador de Rutas";
    int anchoTexto = MeasureText(titulo, 30);
    int posicionX = (GetScreenWidth() - anchoTexto) / 2;
    DrawTextEx(fuentePersonalizada, titulo, { (float)posicionX, 20 }, 30, 0, Color{240, 240, 240, 255});  // Título en color blanco

    int posicionInstruccionesY = 60; // Distancia entre el título y las instrucciones
    DrawTextEx(fuentePersonalizada, instrucciones, {10.0f, (float)posicionInstruccionesY}, 20, 0, Color{190, 190, 190, 255});  // Instrucciones en color gris claro

    // Si estamos en modo edición, mostrar un mensaje adicional
    if (modoEdicion) {
        const char* mensajeModoEdicion = "Modo Edicion Activado! Presiona CTRL para desactivar";
        int anchoMensaje = MeasureText(mensajeModoEdicion, 20);
        int posicionXMensaje = (GetScreenWidth() - anchoMensaje) / 2;
        DrawTextEx(fuentePersonalizada, mensajeModoEdicion, { (float)posicionXMensaje, (float)(INSTRUCCIONES_ALTURA - 30) }, 20, 0, Color{255, 0, 0, 255});  // Aviso en color rojo brillante
    }
}

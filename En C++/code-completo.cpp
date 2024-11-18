#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <unordered_map>
#include <chrono>
#include <iomanip> // Para std::setprecision
#include <ctime>   // Para std::clock()
#include "raylib.h"

// Variables globales para medir tiempos
auto tiempoInicio = std::chrono::high_resolution_clock::now();
auto tiempoBFSInicio = std::chrono::high_resolution_clock::time_point();
auto tiempoDStarInicio = std::chrono::high_resolution_clock::time_point();

// Variables globales para medir tiempo de CPU
std::clock_t cpuInicio = std::clock();
std::clock_t cpuBFSInicio;
std::clock_t cpuDStarInicio;

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOGDI
#define NOUSER
#include <windows.h>
#include <psapi.h>
#else
#include <unistd.h>
#include <sys/resource.h>
#endif

// Función para obtener el uso de memoria actual
size_t getCurrentRSS() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS info;
    GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
    return (size_t)info.WorkingSetSize;
#else
    long rss = 0L;
    FILE* fp = nullptr;
    if ((fp = fopen("/proc/self/statm", "r")) == nullptr)
        return (size_t)0L; /* No se puede abrir */
    if (fscanf(fp, "%*s%ld", &rss) != 1) {
        fclose(fp);
        return (size_t)0L; /* No se puede leer */
    }
    fclose(fp);
    return (size_t)rss * (size_t)sysconf(_SC_PAGESIZE);
#endif
}

const int VACIO = 0;
const int BLOQUEADO = 1;
const int ORIGEN = 2;
const int DESTINO = 3;
const int RUTA = 4;
const int CELDA_TAM = 40;
const int INSTRUCCIONES_ALTURA = 240;  // Altura de la sección de instrucciones

Font fuentePersonalizada;

struct Celda {
    int fila;
    int columna;
    int estado;
    bool visitado;
    Celda* padre;
    float g, h;  // g: coste acumulado, h: heurística
    float f;     // f = g + h (coste total)

    Celda() : fila(-1), columna(-1), estado(VACIO), visitado(false), padre(nullptr), g(INFINITY), h(0), f(INFINITY) {}

    Celda(int f, int c) : fila(f), columna(c), estado(VACIO), visitado(false), padre(nullptr), g(INFINITY), h(0), f(INFINITY) {}

    // Función de comparación para la cola de prioridad (min-heap)
    bool operator>(const Celda& otro) const {
        return f > otro.f;
    }
};

class Grid {
private:
    std::vector<std::vector<Celda>> celdas;
    int filas;
    int columnas;

public:
    Grid(int f, int c) : filas(f), columnas(c) {
        celdas.resize(filas, std::vector<Celda>(columnas));
        for (int i = 0; i < filas; i++) {
            for (int j = 0; j < columnas; j++) {
                celdas[i][j] = Celda(i, j);
            }
        }
    }

    Celda& obtenerCelda(int fila, int columna) {
        return celdas[fila][columna];
    }

    void dibujarGrilla() const {
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
                    
                    // Usamos MeasureTextEx para calcular el ancho del texto
                    Vector2 size = MeasureTextEx(fuentePersonalizada, texto, 20, 0);

                    // Centrar el texto horizontalmente en la celda
                    float posX = posicion.x + (CELDA_TAM - size.x) / 2;  // Centrado horizontal
                    float posY = posicion.y + (CELDA_TAM - size.y) / 2;  // Centrado vertical

                    // Usamos DrawTextEx para dibujar el texto con la fuente personalizada, ya centrado
                    DrawTextEx(fuentePersonalizada, texto, { posX, posY }, 20, 0, BLACK);
                }
            }
        }
    }

    void reiniciar(bool reiniciarBloqueados = false, bool reiniciarOrigenDestino = false) {
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

                // Reiniciamos los valores de la búsqueda para las celdas que no están bloqueadas
                celda.visitado = false;  // Reiniciamos el estado de visita
                celda.padre = nullptr;  // Restablecemos el padre
                celda.g = INFINITY;
                celda.h = 0;
                celda.f = INFINITY;
            }
        }
    }

    bool BFS(Celda& origen, Celda& destino) {
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

            // Revisamos las 4 celdas vecinas (arriba, abajo, izquierda, derecha)
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    // Para BFS, solo exploramos las 4 direcciones cardinales (no diagonales)
                    if (i == 0 || j == 0) {  // Aseguramos que solo sean direcciones cardinales
                        if (i == 0 && j == 0) continue; // No procesamos la celda actual
                        int nuevaFila = actual->fila + i;
                        int nuevaColumna = actual->columna + j;
                        if (i != 0 && j != 0) continue;  // No procesamos las diagonales

                        // Verificamos que la celda esté dentro de los límites y no esté bloqueada
                        if (nuevaFila >= 0 && nuevaFila < filas && nuevaColumna >= 0 && nuevaColumna < columnas) {
                            Celda& vecino = obtenerCelda(nuevaFila, nuevaColumna);
                            if (!vecino.visitado && vecino.estado != BLOQUEADO) {
                                vecino.visitado = true;
                                vecino.padre = actual;  // Guardamos al padre para reconstruir la ruta
                                cola.push(&vecino);  // Añadimos el vecino a la cola
                            }
                        }
                    }
                }
            }
        }

        if (destino.padre != nullptr) {
            Celda* temp = &destino;
            while (temp != nullptr) {
                // Asegurarnos de no cambiar el origen ni el destino a "RUTA"
                if (temp != &origen && temp != &destino) {
                    temp->estado = RUTA;
                }
                temp = temp->padre;
            }
            return true;
        }
        return false;
    }

    bool DStarLite(Celda& origen, Celda& destino) {
        reiniciar();
        
        // Definimos una cola de prioridad para explorar las celdas con el menor coste f
        std::unordered_map<Celda*, float> colaPrioridad;
        
        // Inicializamos la celda origen
        origen.g = 0;
        origen.h = calcularHeuristica(origen, destino);
        origen.f = origen.g + origen.h;
        colaPrioridad[&origen] = origen.f;  // Almacenamos la celda con su valor f

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

            colaPrioridad.erase(actual);  // Eliminamos la celda con el menor coste f

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
        
        return false; // No se encontró una ruta
    }

    float calcularHeuristica(const Celda& origen, const Celda& destino) const {
        // Heurística de Manhattan (distancia en línea recta)
        return abs(origen.fila - destino.fila) + abs(origen.columna - destino.columna);
    }
};

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

int main() {
    const int anchoPantalla = 800;
    const int altoPantalla = 800;
    const int filas = (altoPantalla - INSTRUCCIONES_ALTURA) / CELDA_TAM; // Restamos la altura de las instrucciones
    const int columnas = anchoPantalla / CELDA_TAM;
    bool modoEdicion = false;
    Celda* origen = nullptr;
    Celda* destino = nullptr;

    InitWindow(anchoPantalla, altoPantalla, "Buscador de Rutas");
    // Cargar la fuente
    fuentePersonalizada = LoadFont("DejaVuSans-Bold.ttf");
    Grid grid(filas, columnas);
    SetTargetFPS(60);

    bool cierreVentana = false;
    bool caminoEncontrado = true;

    // Medir el tiempo de carga del programa
    auto tiempoCarga = std::chrono::high_resolution_clock::now() - tiempoInicio;
    // Medir el tiempo de CPU al inicio
    std::clock_t cpuCarga = std::clock() - cpuInicio;

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "\n--- Resultados de Tiempo de Carga ---" << std::endl;
    std::cout << "Tiempo de carga del programa: "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(tiempoCarga).count() / 1e6
              << " ms" << std::endl;
    std::cout << "Tiempo de CPU utilizado en la carga: "
              << (cpuCarga * 1000.0) / CLOCKS_PER_SEC << " ms" << std::endl;

    // Imprimir el uso de memoria inicial
    std::cout << "Uso de memoria RAM inicial del programa: "
              << getCurrentRSS() / 1024 << " KB" << std::endl;

    // Bucle principal
    while (!cierreVentana) {
        if (IsKeyPressed(KEY_LEFT_CONTROL)) {
            modoEdicion = !modoEdicion;
        }

        if (modoEdicion) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                int mouseX = GetMouseX() / CELDA_TAM;
                int mouseY = (GetMouseY() - INSTRUCCIONES_ALTURA) / CELDA_TAM;  // Ajustamos por las instrucciones
                // Verificamos si la celda está dentro de los límites de la grilla
                if (mouseX >= 0 && mouseX < columnas && mouseY >= 0 && mouseY < filas) {
                    Celda& celda = grid.obtenerCelda(mouseY, mouseX);
                    if (celda.estado != ORIGEN && celda.estado != DESTINO) {
                        celda.estado = BLOQUEADO;
                    }
                }
            }
            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                int mouseX = GetMouseX() / CELDA_TAM;
                int mouseY = (GetMouseY() - INSTRUCCIONES_ALTURA) / CELDA_TAM;  // Ajustamos por las instrucciones
                // Verificamos si la celda está dentro de los límites de la grilla
                if (mouseX >= 0 && mouseX < columnas && mouseY >= 0 && mouseY < filas) {
                    Celda& celda = grid.obtenerCelda(mouseY, mouseX);
                    if (celda.estado != ORIGEN && celda.estado != DESTINO) {
                        celda.estado = VACIO;
                    }
                }
            }
        } else {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                int mouseX = GetMouseX() / CELDA_TAM;
                int mouseY = (GetMouseY() - INSTRUCCIONES_ALTURA) / CELDA_TAM;  // Ajustamos por las instrucciones
                // Verificamos si la celda está dentro de los límites de la grilla
                if (mouseX >= 0 && mouseX < columnas && mouseY >= 0 && mouseY < filas) {
                    Celda& celda = grid.obtenerCelda(mouseY, mouseX);
                    if (!origen) {
                        origen = &celda;
                        celda.estado = ORIGEN;
                    } else if (!destino) {
                        destino = &celda;
                        celda.estado = DESTINO;
                    }
                }
            }
        }

        // Medir el tiempo y uso de memoria de BFS al presionar ENTER
        if (IsKeyPressed(KEY_ENTER) && origen && destino) {
            tiempoBFSInicio = std::chrono::high_resolution_clock::now();
            cpuBFSInicio = std::clock();
            grid.reiniciar();
            // Medir uso de memoria antes de BFS
            size_t memoriaAntesBFS = getCurrentRSS();
            if (grid.BFS(*origen, *destino)) {
                caminoEncontrado = true;
                auto tiempoBFSEjecucion = std::chrono::high_resolution_clock::now() - tiempoBFSInicio;
                // Medir uso de memoria después de BFS
                size_t memoriaDespuesBFS = getCurrentRSS();
                // Medir tiempo de CPU usado por BFS
                std::clock_t cpuBFSEjecucion = std::clock() - cpuBFSInicio;

                std::cout << "\n--- Resultados de Tiempo de Ejecucion BFS ---" << std::endl;
                std::cout << "Tiempo de ejecucion: "
                          << std::chrono::duration_cast<std::chrono::nanoseconds>(tiempoBFSEjecucion).count() / 1e6
                          << " ms" << std::endl;
                std::cout << "Tiempo de CPU utilizado: "
                          << (cpuBFSEjecucion * 1000.0) / CLOCKS_PER_SEC << " ms" << std::endl;
                std::cout << "Uso de memoria RAM durante BFS: "
                          << (memoriaDespuesBFS - memoriaAntesBFS) / 1024 << " KB" << std::endl;
                std::cout << "Uso de memoria RAM total despues de BFS: "
                          << memoriaDespuesBFS / 1024 << " KB" << std::endl;
            } else {
                caminoEncontrado = false; // No se encontró camino
                std::cout << "\nNo se ha encontrado un camino con BFS." << std::endl;
            }
        }

        // Medir el tiempo y uso de memoria de D* Lite al presionar ESPACIO
        if (IsKeyPressed(KEY_SPACE) && origen && destino) {
            tiempoDStarInicio = std::chrono::high_resolution_clock::now();
            cpuDStarInicio = std::clock();
            grid.reiniciar();
            // Medir uso de memoria antes de D* Lite
            size_t memoriaAntesDStar = getCurrentRSS();
            if (grid.DStarLite(*origen, *destino)) {
                caminoEncontrado = true;
                auto tiempoDStarEjecucion = std::chrono::high_resolution_clock::now() - tiempoDStarInicio;
                // Medir uso de memoria después de D* Lite
                size_t memoriaDespuesDStar = getCurrentRSS();
                // Medir tiempo de CPU usado por D* Lite
                std::clock_t cpuDStarEjecucion = std::clock() - cpuDStarInicio;

                std::cout << "\n--- Resultados de Tiempo de Ejecucion D* Lite ---" << std::endl;
                std::cout << "Tiempo de ejecucion: "
                          << std::chrono::duration_cast<std::chrono::nanoseconds>(tiempoDStarEjecucion).count() / 1e6
                          << " ms" << std::endl;
                std::cout << "Tiempo de CPU utilizado: "
                          << (cpuDStarEjecucion * 1000.0) / CLOCKS_PER_SEC << " ms" << std::endl;
                std::cout << "Uso de memoria RAM durante D* Lite: "
                          << (memoriaDespuesDStar - memoriaAntesDStar) / 1024 << " KB" << std::endl;
                std::cout << "Uso de memoria RAM total despues de D* Lite: "
                          << memoriaDespuesDStar / 1024 << " KB" << std::endl;
            } else {
                caminoEncontrado = false; // No se encontró camino
                std::cout << "\nNo se ha encontrado un camino con D* Lite." << std::endl;
            }
        }

        if (IsKeyPressed(KEY_ESCAPE)) {
            // Si se presiona ESCAPE, reiniciamos la grilla a su estado inicial
            grid.reiniciar(true, true);
            // También reiniciamos las celdas de origen y destino
            origen = nullptr;
            destino = nullptr;
        }

        if (WindowShouldClose() && !IsKeyPressed(KEY_ESCAPE)) {
            cierreVentana = true;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        dibujarInstrucciones(modoEdicion);
        grid.dibujarGrilla();
        EndDrawing();
    }

    // Descargar la fuente cuando termine el programa
    UnloadFont(fuentePersonalizada);
    CloseWindow();

    return 0;
}

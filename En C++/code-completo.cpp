#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <unordered_map>
#include <chrono>
#include <iomanip> // Para std::setprecision, que ayuda a mostrar tiempos con precisión
#include <ctime>   // Para std::clock(), medir el tiempo de CPU
#include "raylib.h" // Biblioteca para la interfaz gráfica (GUI)

// Variables globales para medir tiempos de ejecución de la aplicación
auto tiempoInicio = std::chrono::high_resolution_clock::now(); // Tiempo de inicio de la ejecución
auto tiempoBFSInicio = std::chrono::high_resolution_clock::time_point(); // Tiempo para BFS
auto tiempoDStarInicio = std::chrono::high_resolution_clock::time_point(); // Tiempo para D* Lite

// Variables globales para medir el uso del tiempo de CPU
std::clock_t cpuInicio = std::clock(); // Tiempo de CPU al inicio
std::clock_t cpuBFSInicio; // Tiempo de CPU para BFS
std::clock_t cpuDStarInicio; // Tiempo de CPU para D* Lite

// Definiciones para plataformas Windows o Unix (Linux/macOS)
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOGDI
#define NOUSER
#include <windows.h>
#include <psapi.h> // Librerías de Windows para obtener el uso de memoria
#else
#include <unistd.h>
#include <sys/resource.h> // Librerías de Unix para obtener el uso de memoria
#endif

// Función para obtener el uso de memoria actual del proceso
size_t getCurrentRSS() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS info;
    GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info)); // Obtener memoria usada por el proceso
    return (size_t)info.WorkingSetSize; // Retornar el uso de memoria
#else
    long rss = 0L;
    FILE* fp = nullptr;
    if ((fp = fopen("/proc/self/statm", "r")) == nullptr)
        return (size_t)0L; // Si no se puede abrir, retornar 0
    if (fscanf(fp, "%*s%ld", &rss) != 1) {
        fclose(fp);
        return (size_t)0L; // Si no se puede leer, retornar 0
    }
    fclose(fp);
    return (size_t)rss * (size_t)sysconf(_SC_PAGESIZE); // Multiplicamos por el tamaño de la página de memoria
#endif
}

// Constantes para representar los diferentes estados de las celdas en la grilla
const int VACIO = 0;
const int BLOQUEADO = 1;
const int ORIGEN = 2;
const int DESTINO = 3;
const int RUTA = 4;
const int CELDA_TAM = 40; // Tamaño de cada celda en píxeles
const int INSTRUCCIONES_ALTURA = 240; // Altura de la sección de instrucciones en la interfaz

// Fuente personalizada para dibujar texto
Font fuentePersonalizada;

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
    Celda() : fila(-1), columna(-1), estado(VACIO), visitado(false), padre(nullptr), g(INFINITY), h(0), f(INFINITY) {}

    // Constructor con fila y columna
    Celda(int f, int c) : fila(f), columna(c), estado(VACIO), visitado(false), padre(nullptr), g(INFINITY), h(0), f(INFINITY) {}

    // Operador de comparación para usar en la cola de prioridad (min-heap)
    bool operator>(const Celda& otro) const {
        return f > otro.f; // La celda con mayor f (coste total) se considera "mayor"
    }
};

// Clase que representa la grilla de celdas
class Grid {
private:
    std::vector<std::vector<Celda>> celdas; // Matriz de celdas
    int filas;
    int columnas;

public:
    // Constructor que crea una grilla de celdas
    Grid(int f, int c) : filas(f), columnas(c) {
        celdas.resize(filas, std::vector<Celda>(columnas)); // Redimensionar la grilla
        for (int i = 0; i < filas; i++) {
            for (int j = 0; j < columnas; j++) {
                celdas[i][j] = Celda(i, j); // Inicializar cada celda con sus coordenadas
            }
        }
    }

    // Método para obtener una celda en una posición específica
    Celda& obtenerCelda(int fila, int columna) {
        return celdas[fila][columna];
    }

    // Método para dibujar la grilla de celdas en la pantalla
    void dibujarGrilla() const {
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
    void reiniciar(bool reiniciarBloqueados = false, bool reiniciarOrigenDestino = false) {
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

    // Función de búsqueda BFS
    bool BFS(Celda& origen, Celda& destino) {
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
                        if (i != 0 && j != 0) continue;  // Aseguramos que no procesamos las diagonales

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
    bool DStarLite(Celda& origen, Celda& destino) {
        reiniciar(); // Restablecer el estado de todas las celdas
        
        // Cola de prioridad que guarda celdas con su coste f (g + h)
        std::unordered_map<Celda*, float> colaPrioridad;
        
        // Inicializamos la celda origen con costes g, h, y f
        origen.g = 0;
        origen.h = calcularHeuristica(origen, destino); // Calculamos la heurística
        origen.f = origen.g + origen.h;
        colaPrioridad[&origen] = origen.f;  // Añadimos la celda origen a la cola de prioridad

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

            colaPrioridad.erase(actual);  // Eliminamos la celda con el menor coste f

            // Si hemos llegado al destino, reconstruimos la ruta
            if (actual == &destino) {
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
    float calcularHeuristica(const Celda& origen, const Celda& destino) const {
        // Heurística de Manhattan (distancia en línea recta)
        return abs(origen.fila - destino.fila) + abs(origen.columna - destino.columna);
    }
};

// Función que dibuja las instrucciones en pantalla
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
        DrawTextEx(fuentePersonalizada, mensajeModoEdicion, { (float)posicionXMensaje, (float)(INSTRUCCIONES_ALTURA - 30) }, 20, 0, Color{255, 0, 0, 255}); // Aviso en color rojo brillante
    }
}

// Función principal que inicializa la ventana y gestiona el flujo del programa
int main() {
    const int anchoPantalla = 800;
    const int altoPantalla = 800;
    const int filas = (altoPantalla - INSTRUCCIONES_ALTURA) / CELDA_TAM; // Restamos la altura de las instrucciones
    const int columnas = anchoPantalla / CELDA_TAM;
    bool modoEdicion = false;
    Celda* origen = nullptr;
    Celda* destino = nullptr;

    // Inicializamos la ventana
    InitWindow(anchoPantalla, altoPantalla, "Buscador de Rutas");

    // Cargar la fuente personalizada
    fuentePersonalizada = LoadFont("DejaVuSans-Bold.ttf");

    // Inicializamos la grilla con el número de filas y columnas
    Grid grid(filas, columnas);
    SetTargetFPS(60); // Establecemos el FPS a 60

    bool cierreVentana = false;
    bool caminoEncontrado = true;

    // Medir el tiempo de carga del programa
    auto tiempoCarga = std::chrono::high_resolution_clock::now() - tiempoInicio;
    std::clock_t cpuCarga = std::clock() - cpuInicio;

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "\n--- Resultados de Tiempo de Carga ---" << std::endl;
    std::cout << "Tiempo de carga del programa: "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(tiempoCarga).count() / 1e6
              << " ms" << std::endl;
    std::cout << "Tiempo de CPU utilizado en la carga: "
              << (cpuCarga * 1000.0) / CLOCKS_PER_SEC << " ms" << std::endl;

    std::cout << "Uso de memoria RAM inicial del programa: "
              << getCurrentRSS() / 1024 << " KB" << std::endl;

    // Bucle principal
    while (!cierreVentana) {
        if (IsKeyPressed(KEY_LEFT_CONTROL)) {
            modoEdicion = !modoEdicion;
        }

        if (modoEdicion) {
            // Lógica de selección de celdas en modo edición
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                int mouseX = GetMouseX() / CELDA_TAM;
                int mouseY = (GetMouseY() - INSTRUCCIONES_ALTURA) / CELDA_TAM;
                if (mouseX >= 0 && mouseX < columnas && mouseY >= 0 && mouseY < filas) {
                    Celda& celda = grid.obtenerCelda(mouseY, mouseX);
                    if (celda.estado != ORIGEN && celda.estado != DESTINO) {
                        celda.estado = BLOQUEADO; // Bloqueamos la celda con clic izquierdo
                    }
                }
            }
            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                int mouseX = GetMouseX() / CELDA_TAM;
                int mouseY = (GetMouseY() - INSTRUCCIONES_ALTURA) / CELDA_TAM;
                if (mouseX >= 0 && mouseX < columnas && mouseY >= 0 && mouseY < filas) {
                    Celda& celda = grid.obtenerCelda(mouseY, mouseX);
                    if (celda.estado != ORIGEN && celda.estado != DESTINO) {
                        celda.estado = VACIO; // Desbloqueamos la celda con clic derecho
                    }
                }
            }
        } else {
            // Lógica de selección de origen y destino fuera de modo edición
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                int mouseX = GetMouseX() / CELDA_TAM;
                int mouseY = (GetMouseY() - INSTRUCCIONES_ALTURA) / CELDA_TAM;
                if (mouseX >= 0 && mouseX < columnas && mouseY >= 0 && mouseY < filas) {
                    Celda& celda = grid.obtenerCelda(mouseY, mouseX);
                    if (!origen) {
                        origen = &celda;
                        celda.estado = ORIGEN; // Establecemos la celda origen
                    } else if (!destino) {
                        destino = &celda;
                        celda.estado = DESTINO; // Establecemos la celda destino
                    }
                }
            }
        }

        // Ejecutamos BFS cuando se presiona ENTER
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
                caminoEncontrado = false; // No se encontró ruta
                std::cout << "\nNo se ha encontrado un camino con BFS." << std::endl;
            }
        }

        // Ejecutamos D* Lite cuando se presiona ESPACIO
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
                caminoEncontrado = false; // No se encontró ruta
                std::cout << "\nNo se ha encontrado un camino con D* Lite." << std::endl;
            }
        }

        // Si se presiona ESCAPE, reiniciamos la grilla y las celdas origen y destino
        if (IsKeyPressed(KEY_ESCAPE)) {
            grid.reiniciar(true, true);
            origen = nullptr;
            destino = nullptr;
        }

        // Si se cierra la ventana, terminamos el programa
        if (WindowShouldClose() && !IsKeyPressed(KEY_ESCAPE)) {
            cierreVentana = true;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        dibujarInstrucciones(modoEdicion);
        grid.dibujarGrilla(); // Dibujamos la grilla de celdas
        EndDrawing();
    }

    // Descargar la fuente cuando termine el programa
    UnloadFont(fuentePersonalizada);
    // Cerrar la ventana
    CloseWindow();

    return 0;
}

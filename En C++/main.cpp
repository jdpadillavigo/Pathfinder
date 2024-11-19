#include <chrono>
#include <iomanip> // Para std::setprecision, que ayuda a mostrar tiempos con precisión
#include <ctime>   // Para std::clock(), medir el tiempo de CPU
#include "entidades.h"

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

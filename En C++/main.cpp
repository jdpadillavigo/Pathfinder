#include <chrono>
#include <iomanip> // Para std::setprecision
#include <ctime>   // Para std::clock()
#include "entidades.h"

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

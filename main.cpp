#include "entidades.h"

int main() {
    const int anchoPantalla = 800;
    const int altoPantalla = 800;
    const int filas = (altoPantalla - INSTRUCCIONES_ALTURA) / CELDA_TAM;  // Restamos la altura de las instrucciones
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
                    } else {
                        if (celda.estado == ORIGEN) {
                            origen = nullptr;
                            celda.estado = VACIO;
                        } else if (celda.estado == DESTINO) {
                            destino = nullptr;
                            celda.estado = VACIO;
                        }
                    }
                }
            }
        }

        if (IsKeyPressed(KEY_ENTER) && origen && destino) {
            grid.reiniciar();
            if (grid.BFS(*origen, *destino)) {}
        }

        if (IsKeyPressed(KEY_SPACE) && origen && destino) {
            grid.reiniciar();
            if (grid.DStarLite(*origen, *destino)) {}
        }

        if (IsKeyPressed(KEY_ESCAPE)) {
            // Si se presiona ESCAPE, reiniciamos la grilla a su estado inicial
            grid.reiniciar(true, true);
            origen = nullptr;  // También reiniciamos las celdas de origen y destino
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

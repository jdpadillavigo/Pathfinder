#include <iostream>
#include <vector>
#include <queue>
#include "raylib.h"

using namespace std;

const int VACIO = 0;
const int BLOQUEADO = 1;
const int ORIGEN = 2;
const int DESTINO = 3;
const int RUTA = 4;
const int CELDA_TAM = 40;
const int INSTRUCCIONES_ALTURA = 240;  // Altura de la sección de instrucciones

struct Celda {
    int fila;
    int columna;
    int estado;
    bool visitado;
    Celda* padre;
    float g, h;

    Celda() : fila(-1), columna(-1), estado(VACIO), visitado(false), padre(nullptr), g(0), h(0) {}

    Celda(int f, int c) : fila(f), columna(c), estado(VACIO), visitado(false), padre(nullptr), g(0), h(0) {}
};

class Grid {
private:
    vector<vector<Celda>> celdas;
    int filas;
    int columnas;

public:
    Grid(int f, int c) : filas(f), columnas(c) {
        celdas.resize(filas, vector<Celda>(columnas));
        for (int i = 0; i < filas; i++) {
            for (int j = 0; j < columnas; j++) {
                celdas[i][j] = Celda(i, j);
            }
        }
    }

    Celda& obtenerCelda(int fila, int columna) {
        return celdas[fila][columna];
    }

    void dibujarGrilla() {
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
                    DrawText(celdas[i][j].estado == ORIGEN ? "O" : "D", posicion.x + CELDA_TAM / 4, posicion.y + CELDA_TAM / 4, 20, BLACK);
                }
            }
        }
    }

    void reiniciar() {
        for (int i = 0; i < filas; i++) {
            for (int j = 0; j < columnas; j++) {
                celdas[i][j].estado = VACIO;
                celdas[i][j].visitado = false;
                celdas[i][j].padre = nullptr;
                celdas[i][j].g = 0;
                celdas[i][j].h = 0;
            }
        }
    }

    bool BFS(Celda& origen, Celda& destino) {
        reiniciar();
        queue<Celda*> cola;
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
                    if (i == 0 && j == 0) continue;
                    int nuevaFila = actual->fila + i;
                    int nuevaColumna = actual->columna + j;
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

        if (destino.padre != nullptr) {
            Celda* temp = &destino;
            while (temp != nullptr) {
                temp->estado = RUTA;
                temp = temp->padre;
            }
            return true;
        }
        return false;
    }

    bool DStarLite(Celda& origen, Celda& destino) {
        reiniciar();
        queue<Celda*> cola;
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
                    if (i == 0 && j == 0) continue;
                    int nuevaFila = actual->fila + i;
                    int nuevaColumna = actual->columna + j;
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

        if (destino.padre != nullptr) {
            Celda* temp = &destino;
            while (temp != nullptr) {
                temp->estado = RUTA;
                temp = temp->padre;
            }
            return true;
        }
        return false;
    }
};

void dibujarInstrucciones() {
    const char* instrucciones = 
        "INSTRUCCIONES:\n\n"
        "1. Presiona CTRL para activar/desactivar el modo edición.\n"
        "2. En modo edición, clic izquierdo bloquea casillas (rojo).\n"
        "3. En modo edición, clic derecho desbloquea casillas (celeste claro).\n"
        "4. Haz clic izquierdo para seleccionar origen (verde) y destino (amarillo).\n"
        "5. Presiona ENTER para buscar ruta (BFS) entre origen y destino.\n"
        "6. Presiona ESPACIO para buscar ruta (D*Lite) entre origen y destino.\n"
        "7. Presiona ESC para reiniciar la grilla a su estado inicial.";

    // Fondo de las instrucciones
    DrawRectangle(0, 0, GetScreenWidth(), INSTRUCCIONES_ALTURA, LIGHTGRAY);  // Fondo gris claro

    // Centrar "Buscador de Rutas" en la parte superior de la pantalla
    const char* titulo = "Buscador de Rutas";
    int anchoTexto = MeasureText(titulo, 30);  // Medir el ancho del texto
    int posicionX = (GetScreenWidth() - anchoTexto) / 2;  // Centrar horizontalmente
    DrawText(titulo, posicionX, 20, 30, BLACK);  // Escribir el texto "Buscador de Rutas"

    int posicionInstruccionesY = 60;  // Distancia entre las instrucciones y el título
    DrawText(instrucciones, 10, posicionInstruccionesY, 20, BLACK);  // Escribir las instrucciones debajo del título
}

int main() {
    const int anchoPantalla = 800;
    const int altoPantalla = 800;
    const int filas = (altoPantalla - INSTRUCCIONES_ALTURA) / CELDA_TAM;  // Restamos la altura de las instrucciones
    const int columnas = anchoPantalla / CELDA_TAM;
    bool modoEdicion = false;
    Celda* origen = nullptr;
    Celda* destino = nullptr;
    InitWindow(anchoPantalla, altoPantalla, "Buscador de Rutas");
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
            grid.reiniciar();
            origen = nullptr;  // También reiniciamos las celdas de origen y destino
            destino = nullptr;
        }

        if (WindowShouldClose() && !IsKeyPressed(KEY_ESCAPE)) {
            cierreVentana = true;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        dibujarInstrucciones();
        grid.dibujarGrilla();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

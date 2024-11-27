import java.lang.management.ManagementFactory;
import java.lang.management.ThreadMXBean;
import java.util.*;

public class Cuadricula {
    private final int filas, columnas;
    private final Celda[][] celdas;
    private PriorityQueue<Celda> openList; // Cola de prioridad para el algoritmo D* Lite
    private Celda inicio, fin;
    private int km = 0; // Variable que representa la cantidad de cambios en el entorno

    // Constructor que inicializa la grilla con el número de filas y columnas especificado
    public Cuadricula(int filas, int columnas) {
        this.filas = filas;
        this.columnas = columnas;
        this.celdas = new Celda[filas][columnas];

        // Inicializar todas las celdas de la grilla
        for (int i = 0; i < filas; i++) {
            for (int j = 0; j < columnas; j++) {
                celdas[i][j] = new Celda(i, j); // Crear una nueva celda en cada posición
            }
        }
    }

    // Obtener la celda en una posición específica (fila, columna)
    public Celda getCelda(int fila, int columna) {
        if (fila >= 0 && fila < filas && columna >= 0 && columna < columnas) {
            return celdas[fila][columna]; // Devolver la celda si está dentro de los límites
        }
        return null; // Devolver null si la posición está fuera de los límites
    }

    // Reiniciar la grilla, con opciones para reiniciar las celdas bloqueadas y de inicio/destino
    public void reiniciarCuadricula(boolean reiniciarBloqueadas, boolean reiniciarInicioFinal) {
        for (int i = 0; i < filas; i++) {
            for (int j = 0; j < columnas; j++) {
                Celda celda = celdas[i][j];
                // Reiniciar las celdas de inicio y destino si se especifica
                if (reiniciarInicioFinal && (celda.getEstado() == Celda.INICIO || celda.getEstado() == Celda.FINAL)) {
                    celda.setEstado(Celda.VACIA);
                }
                // Reiniciar las celdas bloqueadas si se especifica
                if (reiniciarBloqueadas) {
                    if (celda.getEstado() == Celda.BLOQUEADA) {
                        celda.setEstado(Celda.VACIA);
                    }
                }
                // Reiniciar las celdas a su estado inicial
                celda.reiniciar();
            }
        }
    }

    // Algoritmo BFS con mediciones de rendimiento
    public boolean bfs(Celda inicio, Celda fin) {
        reiniciarCuadricula(false, false); // Reiniciar la grilla antes de ejecutar BFS

        // Medir el rendimiento (tiempo, memoria y CPU) antes de ejecutar el algoritmo
        long tiempoInicio = System.nanoTime();
        long memoriaAntes = obtenerUsoMemoria();
        long cpuInicio = obtenerTiempoCPU();

        Queue<Celda> cola = new LinkedList<>(); // Cola para la búsqueda en anchura
        inicio.setVisitada(true); // Marcar la celda de inicio como visitada
        cola.add(inicio); // Agregar la celda de inicio a la cola

        // Comenzar la búsqueda en anchura
        while (!cola.isEmpty()) {
            Celda actual = cola.poll(); // Sacar la celda actual de la cola
            if (actual == fin) {
                reconstruirCaminoBFS(fin, inicio); // Reconstruir el camino encontrado
                medirRendimiento("BFS", tiempoInicio, memoriaAntes, cpuInicio); // Medir rendimiento
                return true; // Camino encontrado
            }

            // Explorar los vecinos de la celda actual
            for (Celda vecino : obtenerVecinos(actual)) { // BFS en 4 direcciones (sin diagonales)
                if (!vecino.isVisitada() && vecino.getEstado() != Celda.BLOQUEADA) {
                    vecino.setVisitada(true); // Marcar como visitado
                    vecino.setPadre(actual); // Establecer la celda actual como el padre del vecino
                    cola.add(vecino); // Agregar el vecino a la cola
                }
            }
        }

        return false; // No se encontró un camino
    }

    // Algoritmo D* Lite con mediciones de rendimiento
    public boolean dStarLite(Celda inicio, Celda fin) {
        reiniciarCuadricula(false, false); // Reiniciar la grilla antes de ejecutar D* Lite
        this.inicio = inicio;
        this.fin = fin;
        km = 0; // Inicializar el contador de cambios en el entorno

        // Medir el rendimiento antes de ejecutar el algoritmo
        long tiempoInicio = System.nanoTime();
        long memoriaAntes = obtenerUsoMemoria();
        long cpuInicio = obtenerTiempoCPU();

        iniciarDStarLite(); // Inicializar el algoritmo D* Lite

        Celda ultimo = inicio;
        // Ejecutar el algoritmo D* Lite hasta llegar al destino
        while (!inicio.equals(fin)) {
            if (inicio.getRhs() == Float.POSITIVE_INFINITY) {
                return false; // No se puede llegar al destino
            }

            List<Celda> sucesores = obtenerSucesores(inicio); // Obtener sucesores del nodo actual
            float minCosto = Float.POSITIVE_INFINITY;
            Celda siguiente = null;

            // Evaluar el sucesor con el menor costo
            for (Celda s : sucesores) {
                float costo = obtenerCosto(inicio, s) + s.getG();
                if (costo < minCosto) {
                    minCosto = costo;
                    siguiente = s;
                }
            }

            if (siguiente == null) {
                return false; // No hay sucesores válidos
            }

            inicio = siguiente;
            // Marcar la celda como parte del camino, si no es ni de inicio ni de destino
            if (inicio.getEstado() != Celda.INICIO && inicio.getEstado() != Celda.FINAL) {
                inicio.setEstado(Celda.CAMINO);
            }

            // Si el entorno cambia, se debe actualizar km y recalcular el camino
            if (entornoCambio()) {
                km += heuristic(ultimo, inicio); // Incrementar el contador de cambios
                ultimo = inicio;
                actualizarNodo(inicio); // Actualizar el nodo en la lista abierta
                procesarOpenList(); // Procesar la lista abierta
            }
        }

        reconstruirCaminoDStarLite(this.inicio, fin); // Reconstruir el camino encontrado
        medirRendimiento("D* Lite", tiempoInicio, memoriaAntes, cpuInicio); // Medir rendimiento
        return true; // Camino encontrado
    }

    // Función para reconstruir el camino en BFS
    private void reconstruirCaminoBFS(Celda fin, Celda inicio) {
        Celda actual = fin;
        while (actual != null && actual != inicio) {
            if (actual.getEstado() != Celda.INICIO && actual.getEstado() != Celda.FINAL) {
                actual.setEstado(Celda.CAMINO); // Marcar la celda como parte del camino
            }
            actual = actual.getPadre(); // Retroceder al nodo padre
        }
    }

    // Función para reconstruir el camino en D* Lite
    private void reconstruirCaminoDStarLite(Celda inicio, Celda fin) {
        Celda actual = inicio;
        while (actual != null && actual != fin) {
            if (actual.getEstado() != Celda.INICIO && actual.getEstado() != Celda.FINAL) {
                actual.setEstado(Celda.CAMINO); // Marcar la celda como parte del camino
            }
            actual = actual.getPadre(); // Retroceder al nodo padre
        }
    }

    // Inicializar los parámetros del algoritmo D* Lite
    private void iniciarDStarLite() {
        openList = new PriorityQueue<>(); // Crear la lista abierta
        fin.setRhs(0); // Establecer el valor de rhs del destino
        fin.setKey(calcularClave(fin)); // Calcular la clave del destino
        openList.add(fin); // Agregar el destino a la lista abierta
        procesarOpenList(); // Procesar la lista abierta
    }

    // Procesar la lista abierta en D* Lite
    private void procesarOpenList() {
        while (!openList.isEmpty() && (openList.peek().compareTo(inicio) < 0 || inicio.getRhs() != inicio.getG())) {
            Celda u = openList.poll(); // Extraer el nodo con la clave más pequeña

            // Actualizar el valor de G de la celda
            if (u.getG() > u.getRhs()) {
                u.setG(u.getRhs());
            } else {
                u.setG(Float.POSITIVE_INFINITY); // No se puede acceder a la celda
                actualizarNodo(u); // Actualizar el nodo
            }
            // Actualizar los predecesores
            for (Celda s : obtenerPredecesores(u)) {
                actualizarNodo(s);
            }
        }
    }

    // Actualizar los valores de un nodo (RHS, G y padres)
    private void actualizarNodo(Celda u) {
        if (!u.equals(fin)) {
            float minRhs = Float.POSITIVE_INFINITY;
            Celda mejorSucesor = null;
            for (Celda s : obtenerSucesores(u)) {
                float costo = obtenerCosto(u, s) + s.getG();
                if (costo < minRhs) {
                    minRhs = costo;
                    mejorSucesor = s;
                }
            }
            u.setRhs(minRhs);
            u.setPadre(mejorSucesor); // Establecer el mejor sucesor como padre
        }
        if (openList.contains(u)) {
            openList.remove(u); // Eliminar el nodo de la lista abierta
        }
        if (u.getG() != u.getRhs()) {
            u.setKey(calcularClave(u)); // Recalcular la clave
            openList.add(u); // Reinsertar el nodo en la lista abierta
        }
    }

    // Calcular la clave de un nodo (usado en D* Lite)
    private double[] calcularClave(Celda u) {
        double min = Math.min(u.getG(), u.getRhs());
        return new double[]{min + heuristic(inicio, u) + km, min}; // Clave basada en G, Rhs y la heurística
    }

    // Función heurística para la distancia entre dos celdas (usando la distancia octile)
    private float heuristic(Celda a, Celda b) {
        // Distancia octile para movimientos en 8 direcciones
        float dx = Math.abs(a.getFila() - b.getFila());
        float dy = Math.abs(a.getColumna() - b.getColumna());
        return Math.max(dx, dy); // Distancia octile
    }

    // Obtener los sucesores de una celda
    private List<Celda> obtenerSucesores(Celda celda) {
        return obtenerVecinos(celda);
    }

    // Obtener los predecesores de una celda
    private List<Celda> obtenerPredecesores(Celda celda) {
        return obtenerVecinos(celda);
    }

    // Comprobar si el entorno ha cambiado (en este caso, siempre retorna false)
    private boolean entornoCambio() {
        return false; // No hay cambios en el entorno en esta implementación
    }

    // Calcular el costo de mover de una celda a otra
    private float obtenerCosto(Celda a, Celda b) {
        if (b.getEstado() == Celda.BLOQUEADA) {
            return Float.POSITIVE_INFINITY; // No se puede mover a una celda bloqueada
        }
        // Si el movimiento es ortogonal, el costo es 1.0
        int dx = Math.abs(a.getFila() - b.getFila());
        int dy = Math.abs(a.getColumna() - b.getColumna());
        if ((dx == 1 && dy == 0) || (dx == 0 && dy == 1)) {
            return 1.0f; // Movimiento ortogonal
        }
        return Float.POSITIVE_INFINITY;
    }

    // Obtener los vecinos de una celda
    private List<Celda> obtenerVecinos(Celda celda) {
        List<Celda> vecinos = new ArrayList<>();
        int fila = celda.getFila();
        int columna = celda.getColumna();

        // Definir las direcciones solo ortogonales (sin diagonales)
        int[][] direcciones = new int[][]{
            {-1, 0},  // Arriba
            {0, -1},  // Izquierda
            {0, 1},   // Derecha
            {1, 0}    // Abajo
        };

        // Agregar los vecinos válidos
        for (int[] dir : direcciones) {
            Celda vecino = getCelda(fila + dir[0], columna + dir[1]);
            if (vecino != null) {
                vecinos.add(vecino);
            }
        }

        return vecinos;
    }

    // Medir el rendimiento de un algoritmo (tiempo, memoria y CPU)
    private void medirRendimiento(String algoritmo, long tiempoInicio, long memoriaAntes, long cpuInicio) {
        long tiempoFin = System.nanoTime();
        long memoriaDespues = obtenerUsoMemoria();
        long cpuFin = obtenerTiempoCPU();

        double tiempoEjecucion = (tiempoFin - tiempoInicio) / 1e6; // Convertir a milisegundos
        long memoriaUsada = (memoriaDespues - memoriaAntes) / 1024; // Convertir a KB
        double cpuUsado = (cpuFin - cpuInicio) / 1e6; // Convertir a milisegundos

        // Mostrar resultados de rendimiento
        System.out.println("\n--- Resultados de Tiempo de Ejecución " + algoritmo + " ---");
        System.out.printf("Tiempo de ejecución: %.3f ms\n", tiempoEjecucion);
        System.out.printf("Tiempo de CPU utilizado: %.3f ms\n", cpuUsado);
        System.out.println("Uso de memoria RAM: " + memoriaUsada + " KB");
    }

    // Obtener el uso de memoria en el sistema
    private long obtenerUsoMemoria() {
        Runtime runtime = Runtime.getRuntime();
        return runtime.totalMemory() - runtime.freeMemory();
    }

    // Obtener el tiempo de CPU utilizado por el hilo actual
    private long obtenerTiempoCPU() {
        ThreadMXBean bean = ManagementFactory.getThreadMXBean();
        return bean.getCurrentThreadCpuTime();
    }

    // Obtener el número de filas de la grilla
    public int getFilas() {
        return filas;
    }

    // Obtener el número de columnas de la grilla
    public int getColumnas() {
        return columnas;
    }
}

import java.lang.management.ManagementFactory;
import java.lang.management.ThreadMXBean;
import java.util.*;

public class Cuadricula {
    private final int filas, columnas;
    private final Celda[][] celdas;
    private PriorityQueue<Celda> openList;
    private Celda inicio, fin;
    private int km = 0;

    public Cuadricula(int filas, int columnas) {
        this.filas = filas;
        this.columnas = columnas;
        this.celdas = new Celda[filas][columnas];

        for (int i = 0; i < filas; i++) {
            for (int j = 0; j < columnas; j++) {
                celdas[i][j] = new Celda(i, j);
            }
        }
    }

    public Celda getCelda(int fila, int columna) {
        if (fila >= 0 && fila < filas && columna >= 0 && columna < columnas) {
            return celdas[fila][columna];
        }
        return null;
    }

    public void reiniciarCuadricula(boolean reiniciarBloqueadas, boolean reiniciarInicioFinal) {
        for (int i = 0; i < filas; i++) {
            for (int j = 0; j < columnas; j++) {
                Celda celda = celdas[i][j];
                if (reiniciarInicioFinal && (celda.getEstado() == Celda.INICIO || celda.getEstado() == Celda.FINAL)) {
                    celda.setEstado(Celda.VACIA);
                }
                if (reiniciarBloqueadas) {
                    if (celda.getEstado() == Celda.BLOQUEADA) {
                        celda.setEstado(Celda.VACIA);
                    }
                }
                celda.reiniciar();
            }
        }
    }

    // Algoritmo BFS con mediciones de rendimiento
    public boolean bfs(Celda inicio, Celda fin) {
        reiniciarCuadricula(false, false);

        long tiempoInicio = System.nanoTime();
        long memoriaAntes = obtenerUsoMemoria();
        long cpuInicio = obtenerTiempoCPU();

        Queue<Celda> cola = new LinkedList<>();
        inicio.setVisitada(true);
        cola.add(inicio);

        while (!cola.isEmpty()) {
            Celda actual = cola.poll();
            if (actual == fin) {
                reconstruirCaminoBFS(fin, inicio); // Utilizamos la función específica para BFS
                medirRendimiento("BFS", tiempoInicio, memoriaAntes, cpuInicio);
                return true;
            }

            for (Celda vecino : obtenerVecinos(actual, false)) { // BFS en 4 direcciones
                if (!vecino.isVisitada() && vecino.getEstado() != Celda.BLOQUEADA) {
                    vecino.setVisitada(true);
                    vecino.setPadre(actual);
                    cola.add(vecino);
                }
            }
        }

        return false;
    }

    // Algoritmo D* Lite con mediciones de rendimiento
    public boolean dStarLite(Celda inicio, Celda fin) {
        reiniciarCuadricula(false, false);
        this.inicio = inicio;
        this.fin = fin;
        km = 0;

        long tiempoInicio = System.nanoTime();
        long memoriaAntes = obtenerUsoMemoria();
        long cpuInicio = obtenerTiempoCPU();

        iniciarDStarLite();

        Celda ultimo = inicio;
        while (!inicio.equals(fin)) {
            if (inicio.getRhs() == Float.POSITIVE_INFINITY) {
                return false; // No hay camino
            }

            List<Celda> sucesores = obtenerSucesores(inicio);
            float minCosto = Float.POSITIVE_INFINITY;
            Celda siguiente = null;

            for (Celda s : sucesores) {
                float costo = obtenerCosto(inicio, s) + s.getG();
                if (costo < minCosto) {
                    minCosto = costo;
                    siguiente = s;
                }
            }

            if (siguiente == null) {
                return false;
            }

            inicio = siguiente;
            if (inicio.getEstado() != Celda.INICIO && inicio.getEstado() != Celda.FINAL) {
                inicio.setEstado(Celda.CAMINO);
            }

            // Si el entorno cambia, se debe actualizar km y recalcular el camino
            if (entornoCambio()) {
                km += heuristic(ultimo, inicio);
                ultimo = inicio;
                actualizarNodo(inicio);
                procesarOpenList();
            }
        }

        reconstruirCaminoDStarLite(this.inicio, fin); // Reconstruir el camino para D* Lite
        // Solo medir rendimiento si encontramos el camino
        medirRendimiento("D* Lite", tiempoInicio, memoriaAntes, cpuInicio);
        return true;
    }

    // Función para reconstruir el camino en BFS
    private void reconstruirCaminoBFS(Celda fin, Celda inicio) {
        Celda actual = fin;
        while (actual != null && actual != inicio) {
            if (actual.getEstado() != Celda.INICIO && actual.getEstado() != Celda.FINAL) {
                actual.setEstado(Celda.CAMINO);
            }
            actual = actual.getPadre();
        }
    }

    // Función para reconstruir el camino en D* Lite
    private void reconstruirCaminoDStarLite(Celda inicio, Celda fin) {
        Celda actual = inicio;
        while (actual != null && actual != fin) {
            if (actual.getEstado() != Celda.INICIO && actual.getEstado() != Celda.FINAL) {
                actual.setEstado(Celda.CAMINO);
            }
            actual = actual.getPadre();
        }
    }

    private void iniciarDStarLite() {
        openList = new PriorityQueue<>();
        fin.setRhs(0);
        fin.setKey(calcularClave(fin));
        openList.add(fin);
        procesarOpenList();
    }

    private void procesarOpenList() {
        while (!openList.isEmpty() && (openList.peek().compareTo(inicio) < 0 || inicio.getRhs() != inicio.getG())) {
            Celda u = openList.poll();
            if (u.getG() > u.getRhs()) {
                u.setG(u.getRhs());
            } else {
                u.setG(Float.POSITIVE_INFINITY);
                actualizarNodo(u);
            }
            for (Celda s : obtenerPredecesores(u)) {
                actualizarNodo(s);
            }
        }
    }

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
            u.setPadre(mejorSucesor);
        }
        if (openList.contains(u)) {
            openList.remove(u);
        }
        if (u.getG() != u.getRhs()) {
            u.setKey(calcularClave(u));
            openList.add(u);
        }
    }

    private double[] calcularClave(Celda u) {
        double min = Math.min(u.getG(), u.getRhs());
        return new double[]{min + heuristic(inicio, u) + km, min};
    }

    private float heuristic(Celda a, Celda b) {
        // Distancia octile para movimientos en 8 direcciones
        float dx = Math.abs(a.getFila() - b.getFila());
        float dy = Math.abs(a.getColumna() - b.getColumna());
        return Math.max(dx, dy);
    }

    private List<Celda> obtenerSucesores(Celda celda) {
        return obtenerVecinos(celda, true); // D* Lite en 8 direcciones
    }

    private List<Celda> obtenerPredecesores(Celda celda) {
        return obtenerVecinos(celda, true); // D* Lite en 8 direcciones
    }

    private boolean entornoCambio() {
        return false;
    }

    private float obtenerCosto(Celda a, Celda b) {
        if (b.getEstado() == Celda.BLOQUEADA) {
            return Float.POSITIVE_INFINITY;
        }
        // Si el movimiento es diagonal, el costo es 1.4; si es ortogonal, es 1.0
        int dx = Math.abs(a.getFila() - b.getFila());
        int dy = Math.abs(a.getColumna() - b.getColumna());
        if (dx == 1 && dy == 1) {
            return 1.4f; // Movimiento diagonal
        } else {
            return 1.0f; // Movimiento ortogonal
        }
    }

    private List<Celda> obtenerVecinos(Celda celda, boolean incluirDiagonales) {
        List<Celda> vecinos = new ArrayList<>();
        int fila = celda.getFila();
        int columna = celda.getColumna();

        int[][] direcciones;
        if (incluirDiagonales) {
            // 8 direcciones
            direcciones = new int[][]{
                {-1, -1}, {-1, 0}, {-1, 1},
                {0, -1},          {0, 1},
                {1, -1},  {1, 0},  {1, 1}
            };
        } else {
            // 4 direcciones
            direcciones = new int[][]{
                {-1, 0},
                {0, -1},        {0, 1},
                {1, 0}
            };
        }

        for (int[] dir : direcciones) {
            Celda vecino = getCelda(fila + dir[0], columna + dir[1]);
            if (vecino != null) {
                vecinos.add(vecino);
            }
        }

        return vecinos;
    }

    private void medirRendimiento(String algoritmo, long tiempoInicio, long memoriaAntes, long cpuInicio) {
        long tiempoFin = System.nanoTime();
        long memoriaDespues = obtenerUsoMemoria();
        long cpuFin = obtenerTiempoCPU();

        double tiempoEjecucion = (tiempoFin - tiempoInicio) / 1e6; // Convertir a milisegundos
        long memoriaUsada = (memoriaDespues - memoriaAntes) / 1024; // Convertir a KB
        double cpuUsado = (cpuFin - cpuInicio) / 1e6; // Convertir a milisegundos

        System.out.println("\n--- Resultados de Tiempo de Ejecucion " + algoritmo + " ---");
        System.out.printf("Tiempo de ejecución: %.3f ms\n", tiempoEjecucion);
        System.out.printf("Tiempo de CPU utilizado: %.3f ms\n", cpuUsado);
        System.out.println("Uso de memoria RAM: " + memoriaUsada + " KB");
    }

    private long obtenerUsoMemoria() {
        Runtime runtime = Runtime.getRuntime();
        return runtime.totalMemory() - runtime.freeMemory();
    }

    private long obtenerTiempoCPU() {
        ThreadMXBean bean = ManagementFactory.getThreadMXBean();
        return bean.getCurrentThreadCpuTime();
    }

    public int getFilas() {
        return filas;
    }

    public int getColumnas() {
        return columnas;
    }
}

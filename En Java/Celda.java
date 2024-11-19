public class Celda implements Comparable<Celda> {
    // Constantes que representan los diferentes estados de la celda
    public static final int VACIA = 0;     // Celda vacía (sin bloqueos o marcador)
    public static final int BLOQUEADA = 1; // Celda bloqueada (no se puede pasar)
    public static final int INICIO = 2;    // Celda de inicio (punto de partida)
    public static final int FINAL = 3;     // Celda de destino (punto de llegada)
    public static final int CAMINO = 4;    // Celda parte del camino encontrado

    // Atributos que representan las propiedades de la celda
    private final int fila, columna; // Posición de la celda en la grilla
    private int estado;              // Estado de la celda (vacía, bloqueada, inicio, final, etc.)
    private boolean visitada;        // Marca si la celda ha sido visitada durante la búsqueda
    private Celda padre;             // Apunta a la celda padre (usado en los algoritmos de búsqueda)
    private float g, rhs, h;         // Valores usados en los algoritmos de búsqueda
    private double[] key;            // Clave usada en el algoritmo D* Lite (con 2 valores)

    // Constructor que inicializa una celda en una posición dada
    public Celda(int fila, int columna) {
        this.fila = fila;
        this.columna = columna;
        this.estado = VACIA;                // Estado inicial es vacío
        this.visitada = false;              // Inicialmente no visitada
        this.padre = null;                  // No tiene padre inicialmente
        this.g = Float.POSITIVE_INFINITY;   // Inicializa el valor de g (costo desde el inicio) como infinito
        this.rhs = Float.POSITIVE_INFINITY; // Inicializa el valor rhs (costo estimado hacia el destino) como infinito
        this.h = 0;                         // Inicializa el valor heurístico h como 0 (no se utiliza en algunas implementaciones)
        this.key = new double[]{Double.POSITIVE_INFINITY, Double.POSITIVE_INFINITY}; // Clave inicial infinita
    }

    // Métodos de acceso a las propiedades de la celda
    public int getFila() { return fila; }
    public int getColumna() { return columna; }
    public int getEstado() { return estado; }
    public void setEstado(int estado) { this.estado = estado; }
    public boolean isVisitada() { return visitada; }
    public void setVisitada(boolean visitada) { this.visitada = visitada; }
    public Celda getPadre() { return padre; }
    public void setPadre(Celda padre) { this.padre = padre; }
    public float getG() { return g; }
    public void setG(float g) { this.g = g; }
    public float getRhs() { return rhs; }
    public void setRhs(float rhs) { this.rhs = rhs; }
    public float getH() { return h; }
    public void setH(float h) { this.h = h; }
    public double[] getKey() { return key; }
    public void setKey(double[] key) { this.key = key; }

    // Método para reiniciar los valores de la celda
    public void reiniciar() {
        this.visitada = false;              // Marca como no visitada
        this.padre = null;                  // Elimina la referencia al padre
        this.g = Float.POSITIVE_INFINITY;   // Reinicia el valor de g
        this.rhs = Float.POSITIVE_INFINITY; // Reinicia el valor de rhs
        this.h = 0;                         // Reinicia el valor heurístico h
        this.key = new double[]{Double.POSITIVE_INFINITY, Double.POSITIVE_INFINITY}; // Reinicia la clave

        // Si la celda no es bloqueada ni es el inicio o final, la reinicia a VACIA
        if (estado != BLOQUEADA && estado != INICIO && estado != FINAL) {
            this.estado = VACIA;
        }
    }

    // Comparador para ordenar las celdas en base a su clave (usado en D* Lite)
    @Override
    public int compareTo(Celda otra) {
        // Compara primero por la primera componente de la clave (key[0])
        if (this.key[0] < otra.key[0]) return -1;
        if (this.key[0] > otra.key[0]) return 1;

        // Si la primera componente es igual, compara por la segunda componente (key[1])
        if (this.key[1] < otra.key[1]) return -1;
        if (this.key[1] > otra.key[1]) return 1;

        // Si ambas componentes son iguales, las celdas son consideradas iguales
        return 0;
    }
}

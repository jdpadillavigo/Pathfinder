public class Celda implements Comparable<Celda> {
    public static final int VACIA = 0;
    public static final int BLOQUEADA = 1;
    public static final int INICIO = 2;
    public static final int FINAL = 3;
    public static final int CAMINO = 4;

    private final int fila, columna;
    private int estado;
    private boolean visitada;
    private Celda padre;
    private float g, rhs, h;
    private double[] key;

    public Celda(int fila, int columna) {
        this.fila = fila;
        this.columna = columna;
        this.estado = VACIA;
        this.visitada = false;
        this.padre = null;
        this.g = Float.POSITIVE_INFINITY;
        this.rhs = Float.POSITIVE_INFINITY;
        this.h = 0;
        this.key = new double[]{Double.POSITIVE_INFINITY, Double.POSITIVE_INFINITY};
    }

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

    public void reiniciar() {
        this.visitada = false;
        this.padre = null;
        this.g = Float.POSITIVE_INFINITY;
        this.rhs = Float.POSITIVE_INFINITY;
        this.h = 0;
        this.key = new double[]{Double.POSITIVE_INFINITY, Double.POSITIVE_INFINITY};
        if (estado != BLOQUEADA && estado != INICIO && estado != FINAL) {
            this.estado = VACIA;
        }
    }

    @Override
    public int compareTo(Celda otra) {
        if (this.key[0] < otra.key[0]) return -1;
        if (this.key[0] > otra.key[0]) return 1;
        if (this.key[1] < otra.key[1]) return -1;
        if (this.key[1] > otra.key[1]) return 1;
        return 0;
    }
}

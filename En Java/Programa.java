import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.lang.management.ManagementFactory;
import java.lang.management.ThreadMXBean;

public class Programa extends JPanel {
    private final Cuadricula cuadricula;
    private Celda inicio, fin;
    private boolean modoEdicion = false;

    private static final int ALTURA_INSTRUCCIONES = 240;
    private static final int MARGEN_SUPERIOR = 40;

    public Programa(int filas, int columnas) {
        this.cuadricula = new Cuadricula(filas, columnas);

        setBackground(new Color(30, 42, 71));

        this.addMouseListener(new MouseAdapter() {
            @Override
            public void mousePressed(MouseEvent e) {
                manejarClick(e);
            }
        });

        this.addKeyListener(new KeyAdapter() {
            @Override
            public void keyPressed(KeyEvent e) {
                manejarTeclas(e);
            }
        });

        this.setFocusable(true);
        this.setPreferredSize(new Dimension(columnas * 40 + 105, filas * 40 + ALTURA_INSTRUCCIONES + MARGEN_SUPERIOR + 25));
    }

    private void manejarClick(MouseEvent e) {
        int margenIzquierdo = calcularMargenIzquierdo();
        int columna = (e.getX() - margenIzquierdo) / 40;
        int fila = (e.getY() - ALTURA_INSTRUCCIONES - MARGEN_SUPERIOR) / 40;

        if (fila < 0 || columna < 0 || columna >= cuadricula.getColumnas() || fila >= cuadricula.getFilas()) return;

        Celda celda = cuadricula.getCelda(fila, columna);
        if (celda == null) return;

        if (modoEdicion) {
            if (SwingUtilities.isLeftMouseButton(e)) {
                if (celda.getEstado() != Celda.INICIO && celda.getEstado() != Celda.FINAL) {
                    celda.setEstado(Celda.BLOQUEADA);
                }
            } else if (SwingUtilities.isRightMouseButton(e)) {
                if (celda.getEstado() != Celda.INICIO && celda.getEstado() != Celda.FINAL) {
                    celda.setEstado(Celda.VACIA);
                }
            }
        } else {
            if (SwingUtilities.isLeftMouseButton(e)) {
                if (inicio == null) {
                    inicio = celda;
                    celda.setEstado(Celda.INICIO);
                } else if (fin == null && celda != inicio) {
                    fin = celda;
                    celda.setEstado(Celda.FINAL);
                }
            }
        }
        repaint();
    }

    private void manejarTeclas(KeyEvent e) {
        switch (e.getKeyCode()) {
            case KeyEvent.VK_CONTROL:
                modoEdicion = !modoEdicion;
                repaint();
                break;
            case KeyEvent.VK_ENTER:
                if (inicio != null && fin != null) {
                    if (cuadricula.bfs(inicio, fin)) {
                        repaint();
                    } else {
                        JOptionPane.showMessageDialog(this, "¡No se encontró un camino con BFS!", "Resultado", JOptionPane.INFORMATION_MESSAGE);
                    }
                }
                break;
            case KeyEvent.VK_SPACE:
                if (inicio != null && fin != null) {
                    if (cuadricula.dStarLite(inicio, fin)) {
                        repaint();
                    } else {
                        JOptionPane.showMessageDialog(this, "¡No se encontró un camino con D* Lite!", "Resultado", JOptionPane.INFORMATION_MESSAGE);
                    }
                }
                break;
            case KeyEvent.VK_ESCAPE:
                reiniciarCuadricula();
                break;
        }
    }

    private void reiniciarCuadricula() {
        inicio = null;
        fin = null;
        cuadricula.reiniciarCuadricula(true, true);
        repaint();
    }

    @Override
    protected void paintComponent(Graphics g) {
        super.paintComponent(g);
        dibujarInstrucciones(g);
        dibujarCuadricula(g);
    }

    private void dibujarInstrucciones(Graphics g) {
        g.setColor(new Color(30, 42, 71));
        g.fillRect(0, 0, getWidth(), ALTURA_INSTRUCCIONES);

        String titulo = "Buscador de Rutas";
        g.setFont(new Font("Arial", Font.BOLD, 24));
        g.setColor(Color.WHITE);
        FontMetrics metrics = g.getFontMetrics();
        int xTitulo = (getWidth() - metrics.stringWidth(titulo)) / 2;
        g.drawString(titulo, xTitulo, 40);

        g.setFont(new Font("Arial", Font.PLAIN, 14));
        g.setColor(Color.LIGHT_GRAY);
        String[] instrucciones = {
            "Instrucciones:",
            "",
            "1. Presiona CTRL para activar/desactivar el modo edición.",
            "2. En modo edición, Clic Izquierdo bloquea casillas (rojo).",
            "3. En modo edición, Clic Derecho desbloquea casillas (celeste claro).",
            "4. Presiona Clic Izquierdo para seleccionar origen (verde) y destino (amarillo).",
            "5. Presiona ENTER para buscar ruta (BFS) entre origen y destino.",
            "6. Presiona ESPACIO para buscar ruta (D* Lite) entre origen y destino.",
            "7. Presiona ESC para reiniciar la grilla a su estado inicial."
        };

        int y = 70;
        for (String linea : instrucciones) {
            g.drawString(linea, 10, y);
            y += 20;
        }

        if (modoEdicion) {
            g.setFont(new Font("Arial", Font.BOLD, 14));
            g.setColor(Color.RED);
            String aviso = "¡Modo Edición Activado! Presiona CTRL para desactivar";
            metrics = g.getFontMetrics();
            g.drawString(aviso, (getWidth() - metrics.stringWidth(aviso)) / 2, ALTURA_INSTRUCCIONES + 20);
        }
    }

    private void dibujarCuadricula(Graphics g) {
        int margenSuperior = ALTURA_INSTRUCCIONES + MARGEN_SUPERIOR;
        int margenIzquierdo = calcularMargenIzquierdo();

        for (int i = 0; i < cuadricula.getFilas(); i++) {
            for (int j = 0; j < cuadricula.getColumnas(); j++) {
                Celda celda = cuadricula.getCelda(i, j);
                Color color;
                switch (celda.getEstado()) {
                    case Celda.VACIA -> color = Color.CYAN;
                    case Celda.BLOQUEADA -> color = Color.RED;
                    case Celda.INICIO -> color = Color.GREEN;
                    case Celda.FINAL -> color = Color.YELLOW;
                    case Celda.CAMINO -> color = Color.BLUE;
                    default -> color = Color.CYAN;
                }
                int x = j * 40 + margenIzquierdo;
                int y = i * 40 + margenSuperior;
                g.setColor(color);
                g.fillRect(x, y, 40, 40);
                g.setColor(Color.BLACK);
                g.drawRect(x, y, 40, 40);

                if (celda.getEstado() == Celda.INICIO) {
                    g.setFont(new Font("Arial", Font.BOLD, 14));
                    g.setColor(Color.BLACK);
                    g.drawString("O", x + 15, y + 25);
                }
                if (celda.getEstado() == Celda.FINAL) {
                    g.setFont(new Font("Arial", Font.BOLD, 14));
                    g.setColor(Color.BLACK);
                    g.drawString("D", x + 15, y + 25);
                }
            }
        }
    }

    private int calcularMargenIzquierdo() {
        return (getWidth() - cuadricula.getColumnas() * 40) / 2;
    }

    public static void main(String[] args) {
        // Inicio de las mediciones de rendimiento
        long tiempoInicioPrograma = System.nanoTime();
        long memoriaAntesPrograma = obtenerUsoMemoria();
        long cpuInicioPrograma = obtenerTiempoCPU();

        JFrame ventana = new JFrame("Buscador de Rutas");
        Programa buscador = new Programa(14, 20);
        ventana.add(buscador);
        ventana.pack();
        ventana.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        ventana.setLocationRelativeTo(null); // Centrar ventana
        ventana.setVisible(true);

        // Fin de las mediciones de rendimiento
        long tiempoFinPrograma = System.nanoTime();
        long memoriaDespuesPrograma = obtenerUsoMemoria();
        long cpuFinPrograma = obtenerTiempoCPU();

        // Cálculo de las métricas
        double tiempoCargaPrograma = (tiempoFinPrograma - tiempoInicioPrograma) / 1e6; // En milisegundos
        long memoriaUsadaPrograma = (memoriaDespuesPrograma - memoriaAntesPrograma) / 1024; // En KB
        double cpuUsadoPrograma = (cpuFinPrograma - cpuInicioPrograma) / 1e6; // En milisegundos

        // Mostrar las métricas
        System.out.println("\n--- Resultados de Tiempo de Carga ---");
        System.out.printf("Tiempo de carga del programa: %.3f ms\n", tiempoCargaPrograma);
        System.out.printf("Tiempo de CPU utilizado en la carga: %.3f ms\n", cpuUsadoPrograma);
        System.out.println("Uso de memoria RAM inicial del programa: " + memoriaUsadaPrograma + " KB");
    }

    // Funciones para obtener el uso de memoria y tiempo de CPU
    private static long obtenerUsoMemoria() {
        Runtime runtime = Runtime.getRuntime();
        return runtime.totalMemory() - runtime.freeMemory();
    }

    private static long obtenerTiempoCPU() {
        ThreadMXBean bean = ManagementFactory.getThreadMXBean();
        if (bean.isCurrentThreadCpuTimeSupported()) {
            return bean.getCurrentThreadCpuTime();
        } else {
            return 0L;
        }
    }
}

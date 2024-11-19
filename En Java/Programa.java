import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.lang.management.ManagementFactory;
import java.lang.management.ThreadMXBean;

public class Programa extends JPanel {
    // Instanciamos la clase Cuadricula que representa la grilla de celdas
    private final Cuadricula cuadricula;
    // Variables para almacenar las celdas de inicio y fin
    private Celda inicio, fin;
    // Modo de edición, que permite bloquear o desbloquear celdas
    private boolean modoEdicion = false;

    // Constantes para el diseño de la interfaz
    private static final int ALTURA_INSTRUCCIONES = 240;
    private static final int MARGEN_SUPERIOR = 40;

    // Constructor que inicializa la grilla y la interfaz gráfica
    public Programa(int filas, int columnas) {
        // Crear una nueva instancia de la clase Cuadricula con el tamaño especificado
        this.cuadricula = new Cuadricula(filas, columnas);

        // Color de fondo del panel
        setBackground(new Color(30, 42, 71));

        // MouseListener para manejar los clics del ratón
        this.addMouseListener(new MouseAdapter() {
            @Override
            public void mousePressed(MouseEvent e) {
                manejarClick(e); // Llama a la función manejarClick cuando se presiona el ratón
            }
        });

        // KeyListener para manejar las teclas presionadas
        this.addKeyListener(new KeyAdapter() {
            @Override
            public void keyPressed(KeyEvent e) {
                manejarTeclas(e); // Llama a la función manejarTeclas cuando se presiona una tecla
            }
        });

        // Hacer que el panel sea enfocable para poder capturar eventos de teclado
        this.setFocusable(true);
        // Establecer el tamaño preferido del panel según las filas y columnas de la grilla
        this.setPreferredSize(new Dimension(columnas * 40 + 105, filas * 40 + ALTURA_INSTRUCCIONES + MARGEN_SUPERIOR + 25));
    }

    // Manejar los clics del ratón para interactuar con las celdas de la grilla
    private void manejarClick(MouseEvent e) {
        // Calcular el margen izquierdo para centrar la grilla
        int margenIzquierdo = calcularMargenIzquierdo();
        // Calcular las coordenadas de la fila y columna
        int columna = (e.getX() - margenIzquierdo) / 40;
        int fila = (e.getY() - ALTURA_INSTRUCCIONES - MARGEN_SUPERIOR) / 40;

        // Asegurarse de que las coordenadas estén dentro de los límites de la grilla
        if (fila < 0 || columna < 0 || columna >= cuadricula.getColumnas() || fila >= cuadricula.getFilas()) return;

        // Obtener la celda correspondiente a la posición clickeada
        Celda celda = cuadricula.getCelda(fila, columna);
        if (celda == null) return;

        // Si estamos en modo edición, podemos bloquear o desbloquear celdas
        if (modoEdicion) {
            if (SwingUtilities.isLeftMouseButton(e)) {
                // Bloquear la celda si no es de inicio ni de destino
                if (celda.getEstado() != Celda.INICIO && celda.getEstado() != Celda.FINAL) {
                    celda.setEstado(Celda.BLOQUEADA);
                }
            } else if (SwingUtilities.isRightMouseButton(e)) {
                // Desbloquear la celda si no es de inicio ni de destino
                if (celda.getEstado() != Celda.INICIO && celda.getEstado() != Celda.FINAL) {
                    celda.setEstado(Celda.VACIA);
                }
            }
        } else {
            // Si no estamos en modo edición, seleccionamos las celdas de inicio y destino
            if (SwingUtilities.isLeftMouseButton(e)) {
                if (inicio == null) {
                    // Si no hay celda de inicio, la configuramos
                    inicio = celda;
                    celda.setEstado(Celda.INICIO);
                } else if (fin == null && celda != inicio) {
                    // Si no hay celda de fin, la configuramos
                    fin = celda;
                    celda.setEstado(Celda.FINAL);
                }
            }
        }
        repaint(); // Volver a dibujar la grilla después de interactuar
    }

    // Manejar las teclas presionadas para ejecutar las acciones de búsqueda o edición
    private void manejarTeclas(KeyEvent e) {
        switch (e.getKeyCode()) {
            case KeyEvent.VK_CONTROL:
                // Alternar entre activar y desactivar el modo edición al presionar CTRL
                modoEdicion = !modoEdicion;
                repaint(); // Redibujar la interfaz para reflejar el cambio
                break;
            case KeyEvent.VK_ENTER:
                // Ejecutar el algoritmo BFS si las celdas de inicio y fin están configuradas
                if (inicio != null && fin != null) {
                    if (cuadricula.bfs(inicio, fin)) {
                        repaint(); // Redibujar la grilla después de calcular el camino
                    } else {
                        // Mostrar un mensaje si no se encontró un camino
                        JOptionPane.showMessageDialog(this, "¡No se encontró un camino con BFS!", "Resultado", JOptionPane.INFORMATION_MESSAGE);
                    }
                }
                break;
            case KeyEvent.VK_SPACE:
                // Ejecutar el algoritmo D* Lite si las celdas de inicio y fin están configuradas
                if (inicio != null && fin != null) {
                    if (cuadricula.dStarLite(inicio, fin)) {
                        repaint(); // Redibujar la grilla después de calcular el camino
                    } else {
                        // Mostrar un mensaje si no se encontró un camino
                        JOptionPane.showMessageDialog(this, "¡No se encontró un camino con D* Lite!", "Resultado", JOptionPane.INFORMATION_MESSAGE);
                    }
                }
                break;
            case KeyEvent.VK_ESCAPE:
                // Reiniciar la grilla y las celdas de inicio y fin al presionar ESC
                reiniciarCuadricula();
                break;
        }
    }

    // Reiniciar la grilla, borrando las celdas de inicio y fin y restableciendo los estados
    private void reiniciarCuadricula() {
        inicio = null;
        fin = null;
        cuadricula.reiniciarCuadricula(true, true); // Limpiar la grilla
        repaint(); // Redibujar la interfaz después del reinicio
    }

    // Método sobrescrito de JPanel para pintar los componentes gráficos
    @Override
    protected void paintComponent(Graphics g) {
        super.paintComponent(g);
        dibujarInstrucciones(g); // Dibujar las instrucciones en la parte superior
        dibujarCuadricula(g); // Dibujar la grilla de celdas
    }

    // Dibujar las instrucciones sobre un fondo en la parte superior de la interfaz
    private void dibujarInstrucciones(Graphics g) {
        g.setColor(new Color(30, 42, 71)); // Establecer el color de fondo para las instrucciones
        g.fillRect(0, 0, getWidth(), ALTURA_INSTRUCCIONES); // Dibujar el fondo

        // Título de la aplicación
        String titulo = "Buscador de Rutas";
        g.setFont(new Font("Arial", Font.BOLD, 24)); // Establecer la fuente del título
        g.setColor(Color.WHITE); // Establecer el color blanco para el texto
        FontMetrics metrics = g.getFontMetrics();
        int xTitulo = (getWidth() - metrics.stringWidth(titulo)) / 2; // Centrar el título
        g.drawString(titulo, xTitulo, 40);

        // Instrucciones detalladas para el usuario
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
        // Dibujar cada línea de instrucciones
        for (String linea : instrucciones) {
            g.drawString(linea, 10, y);
            y += 20;
        }

        // Mensaje de advertencia si el modo edición está activado
        if (modoEdicion) {
            g.setFont(new Font("Arial", Font.BOLD, 14));
            g.setColor(Color.RED);
            String aviso = "¡Modo Edición Activado! Presiona CTRL para desactivar";
            metrics = g.getFontMetrics();
            g.drawString(aviso, (getWidth() - metrics.stringWidth(aviso)) / 2, ALTURA_INSTRUCCIONES + 20);
        }
    }

    // Dibujar la grilla de celdas con sus respectivos colores según el estado
    private void dibujarCuadricula(Graphics g) {
        int margenSuperior = ALTURA_INSTRUCCIONES + MARGEN_SUPERIOR;
        int margenIzquierdo = calcularMargenIzquierdo(); // Calcular el margen izquierdo para centrar

        // Iterar por cada celda de la grilla y dibujarla
        for (int i = 0; i < cuadricula.getFilas(); i++) {
            for (int j = 0; j < cuadricula.getColumnas(); j++) {
                Celda celda = cuadricula.getCelda(i, j);
                Color color;

                // Asignar el color según el estado de la celda
                switch (celda.getEstado()) {
                    case Celda.VACIA -> color = Color.CYAN;
                    case Celda.BLOQUEADA -> color = Color.RED;
                    case Celda.INICIO -> color = Color.GREEN;
                    case Celda.FINAL -> color = Color.YELLOW;
                    case Celda.CAMINO -> color = Color.BLUE;
                    default -> color = Color.CYAN;
                }

                // Dibujar el rectángulo de la celda en la grilla
                int x = j * 40 + margenIzquierdo;
                int y = i * 40 + margenSuperior;
                g.setColor(color);
                g.fillRect(x, y, 40, 40); // Rellenar la celda con el color correspondiente
                g.setColor(Color.BLACK);
                g.drawRect(x, y, 40, 40); // Dibujar el borde de la celda

                // Dibujar las letras 'O' para inicio y 'D' para destino
                if (celda.getEstado() == Celda.INICIO) {
                    g.setFont(new Font("Arial", Font.BOLD, 14));
                    g.setColor(Color.BLACK);
                    g.drawString("O", x + 15, y + 25); // 'O' para origen
                }
                if (celda.getEstado() == Celda.FINAL) {
                    g.setFont(new Font("Arial", Font.BOLD, 14));
                    g.setColor(Color.BLACK);
                    g.drawString("D", x + 15, y + 25); // 'D' para destino
                }
            }
        }
    }

    // Calcular el margen izquierdo para centrar la grilla
    private int calcularMargenIzquierdo() {
        return (getWidth() - cuadricula.getColumnas() * 40) / 2;
    }

    // Método principal que inicializa la ventana y mide el rendimiento del programa
    public static void main(String[] args) {
        // Medir el tiempo de inicio, memoria y CPU antes de ejecutar el programa
        long tiempoInicioPrograma = System.nanoTime();
        long memoriaAntesPrograma = obtenerUsoMemoria();
        long cpuInicioPrograma = obtenerTiempoCPU();

        // Crear la ventana y agregar la instancia del programa
        JFrame ventana = new JFrame("Buscador de Rutas");
        Programa buscador = new Programa(14, 20); // Crear la instancia de Programa con 14 filas y 20 columnas
        ventana.add(buscador);
        ventana.pack();
        ventana.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        ventana.setLocationRelativeTo(null); // Centrar ventana
        ventana.setVisible(true);

        // Medir el tiempo, memoria y CPU al finalizar el programa
        long tiempoFinPrograma = System.nanoTime();
        long memoriaDespuesPrograma = obtenerUsoMemoria();
        long cpuFinPrograma = obtenerTiempoCPU();

        // Calcular el rendimiento del programa
        double tiempoCargaPrograma = (tiempoFinPrograma - tiempoInicioPrograma) / 1e6; // En milisegundos
        long memoriaUsadaPrograma = (memoriaDespuesPrograma - memoriaAntesPrograma) / 1024; // En KB
        double cpuUsadoPrograma = (cpuFinPrograma - cpuInicioPrograma) / 1e6; // En milisegundos

        // Mostrar los resultados de rendimiento
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

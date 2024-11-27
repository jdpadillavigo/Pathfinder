<h1 align="center">Pathfinder</h1>
<p align="center">
    <a>
        <img src="https://img.shields.io/badge/release-v1.2.2-red" alt="Latest release"/>
    </a>
</p>

## <picture><img src= "https://cdn.discordapp.com/emojis/866463618916024340.gif" width= 20px></picture> About
This project involved implementing an application to test and compare two pathfinding algorithms: BFS (Breadth-First Search) and D* Lite. The application was developed in C++ and Java, using Raylib for the graphical user interface in C++ and Swing in Java. The main objective was to evaluate the performance and memory usage of both languages.

## <picture><img src= "https://cdn.discordapp.com/emojis/866463618916024340.gif" width= 20px></picture> Main features

### 1. Graphical User Interface (GUI)
- The application features a grid containing multiple cells.
- The user can select a start point (green colour) and a destination (yellow colour).

### 2. Edit mode
- Activated or deactivated by pressing CTRL. In this mode, the user can block (red colour) or unblock (light blue colour) cells.

### 3. Pathfinding algorithms
- BFS: When pressing ENTER, the shortest path is calculated and coloured blue.
- D* Lite: When pressing SPACE, the shortest path is calculated and coloured blue.

### 4. Performance and memory usage
- The application displays the performance and memory usage in the Terminal when the program is loaded and any pathfinding algorithm is executed.

### 5. Error handling
- If no path is found between the start and destination, the application shows a message to the user indicating that no path can be found.

### 6. Reset
- Pressing ESC resets the grid to its initial state.
all: build

build:
	g++ -c entidades.cpp -I"./include"
	g++ -c main.cpp -I"./include"
	g++ entidades.o main.o -o main -L"./lib" -lraylib -lopengl32 -lgdi32 -lwinmm
	.\main.exe

clean:
	rm -rf *.exe
	rm -rf *.o

code:
	g++ code-completo.cpp -o main -I"./include" -L"./lib" -lraylib -lopengl32 -lgdi32 -lwinmm
	.\main.exe

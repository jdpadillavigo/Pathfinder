all: build

build:
	g++ main.cpp -o main -I"./include" -L"./lib" -lraylib -lopengl32 -lgdi32 -lwinmm
	.\main.exe

clean:
	rm -rf *.exe
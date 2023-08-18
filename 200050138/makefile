CC=g++

LFLAGS1 = -lsfml-graphics -lsfml-window -lsfml-system
LFLAGS2 = -lSDL2

Mandelbrot_sfml: Mandelbrot_sfml.cpp
	$(CC) Mandelbrot_sfml.cpp -o Mandelbrot_sfml $(LFLAGS1)
	
Application: Application.cpp
	$(CC) Application.cpp -o Application $(LFLAGS2) 

all:
	$(CC) Mandelbrot_sfml.cpp -o Mandelbrot_sfml $(LFLAGS1)
	$(CC) Application.cpp -o Application $(LFLAGS2)
	
.PHONY: clean

BINARIES = Application Mandelbrot_sfml

clean:
	rm -f $(BINARIES)
	

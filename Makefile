FLAGS = -march=native -O3
SFML_FLAGS = -lsfml-graphics -lsfml-window -lsfml-system

start: main
	g++ main.o -o main $(SFML_FLAGS)

main:
	g++ -c $(FLAGS) main.cpp

clean:
	rm -rf main.o main
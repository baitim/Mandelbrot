start: main
	g++ main.o -o main -lsfml-graphics -lsfml-window -lsfml-system

main:
	g++ -c main.cpp

clean:
	rm -rf main.o main
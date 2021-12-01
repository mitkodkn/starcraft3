all: main

main.o: main.c
main: main.o
	gcc -pthread -o starcraft3 main.o

clean:
	rm -f *.o *.zip *~ starcraft3

zip:
	zip starcraft3 *.c *.h Makefile

check-leaks: all
	valgrind --leak-check=full ./starcraft3 10
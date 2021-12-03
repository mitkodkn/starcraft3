all: main

controls.o: controls.c types.h threads.h controls.h

threads.o: types.h controls.h threads.h threads.c

utility.o: types.h utility.h utility.c

main.o: main.c types.h threads.h controls.h utility.h

main: main.o controls.o threads.o utility.o
	gcc -pthread -o starcraft3 main.o controls.o threads.o utility.o

clean:
	rm -f *.o *.zip *~ starcraft3

zip:
	zip starcraft3 *.c *.h Makefile

check-leaks: all
	valgrind --leak-check=full ./starcraft3 10
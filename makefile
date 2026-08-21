Quoridor: main.o board.o
	gcc main.o board.o -o Quoridor

main.o: main.c board.h
	gcc -c main.c -o main.o

board.o: board.c board.h
	gcc -c board.c -o board.o

clean:
	rm *.o
	rm Quoridor
Quoridor: main.o
	gcc main.o -o Quoridor

main.o: main.c
	gcc -c main.c -o main.o

clean:
	rm *.o
	rm Quoridor
Quoridor: main.o board.o bot.o
	gcc main.o board.o bot.o -o Quoridor

main.o: main.c board.h bot.h
	gcc -O2 -c main.c -o main.o

board.o: board.c board.h
	gcc -O2 -c board.c -o board.o

bot.o: bot.c bot.h board.h
	gcc -O2 -c bot.c -o bot.o

clean:
	rm *.o
	rm Quoridor
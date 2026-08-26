all: Quoridor matchMaking/Referee matchMaking/Bot

Quoridor: engine.o board.o bot.o
	gcc engine.o board.o bot.o matchMaking/protocols.o -o Quoridor

engine.o: engine.c board.h bot.h
	gcc -O2 -c engine.c -o engine.o

board.o: board.c board.h
	gcc -O2 -c board.c -o board.o

bot.o: bot.c bot.h board.h
	gcc -O2 -c bot.c -o bot.o



matchMaking/Referee: board.o matchMaking/referee.o matchMaking/protocols.o
	gcc board.o matchMaking/referee.o matchMaking/protocols.o -o matchMaking/Referee

matchMaking/referee.o: matchMaking/referee.c matchMaking/protocols.h
	gcc -c matchMaking/referee.c -o matchMaking/referee.o


matchMaking/Bot: board.o bot.o matchMaking/protocols.o matchMaking/botProtocol.o
	gcc board.o bot.o matchMaking/protocols.o matchMaking/botProtocol.o -o matchMaking/Bot

matchMaking/botProtocol.o: matchMaking/bot_v1.c
	gcc -c matchMaking/bot_v1.c -o matchMaking/botProtocol.o


matchMaking/protocols.o: matchMaking/protocols.c board.h
	gcc -c matchMaking/protocols.c -o matchMaking/protocols.o

clean:
	rm *.o
	rm Quoridor
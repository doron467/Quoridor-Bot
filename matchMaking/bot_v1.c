#include "../board.h"
#include "../bot.h"
#include "protocols.h"

#include <stdio.h>

int main(void)
{
    Board board;
    pBot bot = createBot(&board);

    while (readBoardFEN(&board)) {

        Move move;

        getBestMove(bot, &move);

        printf(
            "%d %d %d\n",
            move.moveType,
            move.b1,
            move.b2
        );

        fflush(stdout);
    }

    return 0;
}
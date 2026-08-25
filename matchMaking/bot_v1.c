#include "../board.h"
#include "../bot.h"
#include "protocols.h"

#include <stdio.h>

int main(void)
{
    Board board;

    while (readBoardFEN(&board)) {

        Move move;

        getBestMove(&board, &move);

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
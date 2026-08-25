#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#include "protocols.h"

int main(void)
{
    Board board;
    Move move;

    while (1) {

        if(!readBoardFEN(&board)){
            break;
        }  

        if (!readMove(&move)) {
            break;
        }

        if (!isLegalMove(&board, &move)) {

            printf("ILLEGAL\n");
            fflush(stdout);
            continue;
        }

        makeMove(&board, &move);

        if (isGameOver(&board)) {

            int winner =
                board.p1pos / 9 == 8 ? 1 : 2;

            printf("WIN %d\n", winner);

        } else {

            printBoardFEN(&board);
        }

        fflush(stdout);
    }

    return 0;
}


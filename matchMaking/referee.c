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

            int bfs1 = bfs(board.hWalls,board.vWalls,board.p1pos,0);
            int bfs2 = bfs(board.hWalls,board.vWalls,board.p2pos,8);

            printf("WIN %d DIFFERENCE %d\n", winner,bfs1 - bfs2);

        } else {

            printBoardFEN(&board);
        }

        fflush(stdout);
    }

    return 0;
}


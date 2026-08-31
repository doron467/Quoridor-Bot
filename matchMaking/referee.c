#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#include "protocols.h"
#include "../bot.h"

int main(void)
{
    Board board;
    pBot bot = createBot(&board);
    Move move;

    while (1) {

        if(!readBoardFEN(&board)){
            break;
        }  

        if (!readMove(&move)) {
            break;
        }

        int8_t movementBuffer[10];
        getPlayerMoves(&board,movementBuffer);
        if (!isLegalMove(bot, &move,movementBuffer,0)) {

            printf("ILLEGAL\n");
            fflush(stdout);
            continue;
        }

        makeMove(bot, &move,0);

        if (isGameOver(&board)) {

            int winner =
                board.p1pos / 9 == 8 ? 1 : 2;

            int bfs1 = bfs(board.hWalls,board.vWalls,board.p1pos,0,NULL);
            int bfs2 = bfs(board.hWalls,board.vWalls,board.p2pos,8,NULL);

            printf("WIN %d DIFFERENCE %d\n", winner,bfs1 - bfs2);

        } else {

            printBoardFEN(&board);
        }

        fflush(stdout);
    }

    destroyBot(bot);

    return 0;
}


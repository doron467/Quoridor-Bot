#include "board.h"
#include "bot.h"
#include "matchMaking/protocols.h"

#include <stdio.h>
#include <ctype.h>
#include <time.h>

#define CLEAR_BUFFER() while (getc(stdin) != '\n');

int main(int argc, char **argv){

    srand(time(NULL));
    Move movesBuffer[1000];
    size_t nextMoveIndex = 0;

    //printf("size: %lu\n",sizeof(Board));
    pBoard board = initializeBoard(argc > 1 ? argv[1][0] - '0' : 1);
    pBot bot = createBot(board);
    uint8_t lastTurn = 0;
    while (1){

        calculateRootPath(bot);

        if (lastTurn != board->turn){
            printBoard(board);
            lastTurn = board->turn;
        }

        printf("enter next move: ");

        char action = getc(stdin);
        if (action == '\n'){continue;}

        if (action == 'i' || action == 'r' || action == 'd'){

            if (action == 'i'){
                
                Move bestMove;
                getBestMove(bot,&bestMove);
                printf("bot's move: ");
                printMove(&bestMove);

            } else if (action == 'r'){

                if (nextMoveIndex > 0){
                    unmakeMove(board,&movesBuffer[--nextMoveIndex]);
                } else {
                    printf("no moves to unmake\n");
                }

            } else if (action == 'd'){
                printf("board state: ");
                printBoardFEN(board);
            }

            CLEAR_BUFFER();
            continue;
        }

        char row = getc(stdin);
        if (row == '\n'){continue;}
        char column = getc(stdin);
        if (column == '\n'){continue;}

        if (isdigit(row) && isdigit(column)){
            row -= '0';
            column -= '0';

            if (row >= 0 && row < 9 && column >= 0 && column < 9){

                if (action == 'm'){
                    int8_t buffer[10]; 
                    int8_t pos = row * 9 + column;
                    getPlayerMoves(board,buffer);
                    bool valid = false;
                    for (int i = 0; buffer[i] != -1; i++){
                        if (buffer[i] == pos){
                            valid = true;
                            break;
                        }
                    }

                    if (valid){
                        Move move = {MOVEMENT,board->turn == 1 ? board->p1pos : board->p2pos, pos};
                        makeMove(board,&move,0);
                        movesBuffer[nextMoveIndex++] = move;

                    } else {
                        printf("invalid move\n");
                    }

                } else if (action == 'h' || action == 'v'){
                    int8_t pos = row * 8 + column;
                    if (canPlaceWall(bot,pos,action == 'h',0)){
                        Move move = {action == 'h' ? HORIZONTAL : VERTICAL,pos,0};
                        makeMove(board,&move,0);
                        movesBuffer[nextMoveIndex++] = move;
                    } else {
                        printf("invalid wall placement\n");
                    }
                }

            } else {
                printf("row and column must be in range\n");
            }

        } else {
            printf("row and column must be digits\n");
        }

         CLEAR_BUFFER();

    }

    destroyBot(bot);
    free(board);

    return 0;
}
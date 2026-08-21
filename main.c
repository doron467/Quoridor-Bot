#include <stdio.h>
#include "board.h"
#include <ctype.h>

int main(int argc, char **argv){

    //printf("size: %lu\n",sizeof(Board));
    pBoard board = initializeBoard();
    uint8_t lastTurn = 2;
    while (1){

        if (lastTurn != board->turn){
            printBoard(board);
            lastTurn = board->turn;
        }

        printf("enter next move: ");

        char c = getc(stdin);
        if (c == 'm'){

            int8_t pos = board->turn == 1 ? board->p1pos : board->p2pos;
            c = getc(stdin);
            if (c == 'l'){
                if (CAN_LEFT(pos)){
                    pos = LEFT(pos);
                    if (board->turn == 1){board->p1pos=pos;} else {board->p2pos = pos;}
                    SWITCH_TURNS(board);
                } else {
                    printf("invalid move\n");
                }
            } else if (c == 'r'){
                if (CAN_RIGHT(pos)){
                    pos = RIGHT(pos);
                    if (board->turn == 1){board->p1pos=pos;} else {board->p2pos = pos;}
                    SWITCH_TURNS(board);
                } else {
                    printf("invalid move\n");
                }
            } else if (c == 'u'){
                if (CAN_UP(pos)){
                    pos = UP(pos);
                    if (board->turn == 1){board->p1pos=pos;} else {board->p2pos = pos;}
                    SWITCH_TURNS(board);
                } else {
                    printf("invalid move\n");
                }
            } else if (c == 'd'){
                if (CAN_DOWN(pos)){
                    pos = DOWN(pos);
                    if (board->turn == 1){board->p1pos=pos;} else {board->p2pos = pos;}
                    SWITCH_TURNS(board);
                } else {
                    printf("invalid move\n");
                }
            } else {
                printf("invalid move\n");
                if (c == '\n'){continue;}
            }

        } else if (c == 'h' || c == 'v') {
            bool horizontal = c == 'h';
            c = getc(stdin);
            if (isdigit(c)){
                int tens = (int) (c - '0');
                c = getc(stdin);
                if (isdigit(c)){

                    int ones = (int) (c - '0');
                    int pos = tens * 10 + ones;
                    if (placeWall(board,pos,horizontal)){
                        SWITCH_TURNS(board);
                    } else {
                        printf("invalid wall placement\n");
                    }

                } else {
                    printf("invalid move\n");
                    if (c == '\n'){continue;}
                }

            } else {
                printf("invalid move\n");
                if (c == '\n'){continue;}
            }

        } else {
            printf("invalid move\n");
            if (c == '\n'){continue;}
        }

        while (getc(stdin) != '\n'); // clear buffer

    }

    return 0;
}
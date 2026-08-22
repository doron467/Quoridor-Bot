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

        char action = getc(stdin);
        if (action == '\n'){continue;}
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
                        if (board->turn == 1){board->p1pos = pos;} else {board->p2pos = pos;}
                        SWITCH_TURNS(board);
                    } else {
                        printf("invalid move\n");
                    }

                } else if (action == 'h' || action == 'v'){
                    int8_t pos = row * 8 + column;
                    if (placeWall(board,pos,action == 'h')){
                        if (board->turn == 1){board->p1wc--;} else {board->p2wc--;}
                        SWITCH_TURNS(board);
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

        while (getc(stdin) != '\n'); // clear buffer

    }

    return 0;
}
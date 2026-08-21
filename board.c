#include <stdlib.h>
#include <stdio.h>
#include "board.h"

pBoard initializeBoard(){

    pBoard gameBoard = malloc(sizeof(Board));
    if (gameBoard == NULL){
        printf("gameboard malloc failed\n");
        exit(1);
    }

    gameBoard->p1pos = P1_START_POS;
    gameBoard->p2pos = P2_START_POS;
    gameBoard->p1wc = INITIAL_WALL_COUNT;
    gameBoard->p2wc = INITIAL_WALL_COUNT;
    gameBoard->hWalls = 0;
    gameBoard->vWalls = 0;
    gameBoard->turn = 1;
    return gameBoard;
}

void printBoard(pBoard board){
    char buffer[1000];
    int j = 0;
    for (int i = 0; i < 81; i++){
        if (i % 9 == 0){buffer[j++] = '\n';}
        char c = '*';
        if (i == board->p1pos){c = '1';}
        if (i == board->p2pos){c = '2';}
        buffer[j++] = ' ';
        buffer[j++] = c;
        buffer[j++] = ' ';
    }
    buffer[j] = '\0';

    printf("%s\n",buffer);
    printf("player1 walls count: %d\n",board->p1wc);
    printf("player2 walls count: %d\n",board->p2wc);
    printf("turn: p%d\n",board->turn);
}


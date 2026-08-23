#include "bot.h"
#include <stdio.h>
#include <math.h>

#define INF 100000

int evaluate(pBoard board){

    //printf("evaluate called\n");

    if (isGameOver(board)){
        // since the player just moved to the end goal, it is now the other player's turn
        // so return negative infinity to show he lost
        return -INF;
    }

    int d1 = bfs(board->hWalls,board->vWalls,board->p1pos,0);
    int d2 = bfs(board->hWalls,board->vWalls,board->p2pos,8);
    int difference = (board->turn == 1) ? (d2 - d1) : (d1 - d2);
    return difference;
}

int negamax(pBoard board, int depth, Move *bestMove);

void tryMove(pBoard board,Move *move,int depth,int *bestScore,Move *bestMove){
    makeMove(board,move);
    int score = -negamax(board,depth-1,NULL);
    unmakeMove(board,move);

    if (score > *bestScore){
        *bestScore = score;

        if (bestMove != NULL){
            *bestMove = *move;
        }
    }
}

int negamax(pBoard board, int depth, Move *bestMove){

    if (depth == 0 || isGameOver(board)){
        return evaluate(board);
    }

    int bestScore = -INF;
    int8_t movementBuffer[10];
    Move currentMove;

    // search movements
    getPlayerMoves(board,movementBuffer);
    currentMove.moveType = MOVEMENT;
    currentMove.b1 = (board->turn == 1) ? board->p1pos : board->p2pos;
    for (int i = 0; movementBuffer[i] != -1; i++){
        currentMove.b2 = movementBuffer[i];
        tryMove(board,&currentMove,depth,&bestScore,bestMove);
    }

    // search wall placements
    currentMove.moveType = HORIZONTAL;
    for (int8_t i = 0; i < 64; i++){
        if (canPlaceWall(board,i,true)){
            currentMove.b1 = i;
            tryMove(board,&currentMove,depth,&bestScore,bestMove);
        }
    }

    currentMove.moveType = VERTICAL;
    for (int8_t i = 0; i < 64; i++){
        if (canPlaceWall(board,i,false)){
            currentMove.b1 = i;
            tryMove(board,&currentMove,depth,&bestScore,bestMove);
        }
    }

    return bestScore;
}

void getBestMove(pBoard board,Move *move){
    negamax(board,3,move);
}
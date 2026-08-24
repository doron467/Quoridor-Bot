#include "bot.h"

#include <stdio.h>
#include <math.h>
#include <time.h>

#define INF 100000
#define BIGGER_INF ((INF) * 2)

int evaluate(pBoard board){

    //printf("evaluate called\n");

    if (isGameOver(board)){
        // since the player just moved to the end goal, it is now the other player's turn
        // so return negative infinity to show he lost
        return -INF;
    }

    int d1 = board->d1;
    int d2 = board->d2;
    if (!board->updated){
        d1 = bfs(board->hWalls,board->vWalls,board->p1pos,0);
        d2 = bfs(board->hWalls,board->vWalls,board->p2pos,8);
    }
    
    int difference = (board->turn == 1) ? (d2 - d1) : (d1 - d2);
    return difference + 1;
}

int negamax(pBoard board, int depth, Move *bestMove,int alpha,int beta);

void tryMove(pBoard board,Move *move,int depth,int *alpha,int *beta,Move *bestMove){
    makeMove(board,move);
    int score = -negamax(board,depth-1,NULL,-*beta,-*alpha);
    unmakeMove(board,move);

    if (score > *alpha){
        *alpha = score;

        if (bestMove != NULL){
            *bestMove = *move;
        }
    }


}

int negamax(pBoard board, int depth, Move *bestMove,int alpha,int beta){

    if (depth == 0 || isGameOver(board)){
        return evaluate(board);
    }

    int8_t movementBuffer[10];
    Move currentMove;

    // search movements
    getPlayerMoves(board,movementBuffer);
    currentMove.moveType = MOVEMENT;
    currentMove.b1 = (board->turn == 1) ? board->p1pos : board->p2pos;
    for (int i = 0; movementBuffer[i] != -1; i++){
        currentMove.b2 = movementBuffer[i];
        tryMove(board,&currentMove,depth,&alpha,&beta,bestMove);
        if (alpha >= beta){return alpha;}
    }

    // search wall placements
    currentMove.moveType = HORIZONTAL;
    for (int8_t i = 0; i < 64; i++){
        if (canPlaceWall(board,i,true)){
            currentMove.b1 = i;
            tryMove(board,&currentMove,depth,&alpha,&beta,bestMove);
            if (alpha >= beta){return alpha;}
        }
    }

    currentMove.moveType = VERTICAL;
    for (int8_t i = 0; i < 64; i++){
        if (canPlaceWall(board,i,false)){
            currentMove.b1 = i;
            tryMove(board,&currentMove,depth,&alpha,&beta,bestMove);
            if (alpha >= beta){return alpha;}
        }
    }

    return alpha;
}

void getBestMove(pBoard board,Move *move){
    clock_t start = clock();
    int score = negamax(board,5,move,-BIGGER_INF,BIGGER_INF);
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;

    printf("bot evaluation: %d\n",score);
    printf("computation time: %f\n",elapsed);
}
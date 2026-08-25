#define INF 100000
#define BIGGER_INF ((INF) * 2)

#include "bot.h"

#include <stdio.h>
#include <math.h>
#include <time.h>

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

int negamax(pBoard board, int depth,int alpha,int beta, Deadline *deadline);

void tryMove(pBoard board,Move *move,int depth,int *alpha,int *beta,Deadline *deadline){
    makeMove(board,move);
    int score = -negamax(board,depth-1,-*beta,-*alpha,deadline);
    unmakeMove(board,move);

    if (score > *alpha){
        *alpha = score;
    }
}

int negamax(pBoard board, int depth,int alpha,int beta, Deadline *deadline){

    if ((deadline->nodesCount++) == CLOCK_CHECK){
        deadline->nodesCount = 0;
        if (clock() > deadline->deadline){
            deadline->timeExpired = true;
            return 0;
        }
    }

    if (depth == 0 || isGameOver(board)){
        return evaluate(board);
    }

    int8_t movementBuffer[10];
    Move currentMove;

    // if (bestMove != NULL && bestMove->moveType != NULL_MOVE){
    //     currentMove = *bestMove;
    //     tryMove(board,&currentMove,depth,&alpha,&beta,bestMove,deadline);
    //     if (deadline->timeExpired){return 0;}
    // }

    // search movements
    getPlayerMoves(board,movementBuffer);
    currentMove.moveType = MOVEMENT;
    currentMove.b1 = (board->turn == 1) ? board->p1pos : board->p2pos;
    for (int i = 0; movementBuffer[i] != -1; i++){
        currentMove.b2 = movementBuffer[i];
        tryMove(board,&currentMove,depth,&alpha,&beta,deadline);
        if (alpha >= beta){return alpha;}
        if (deadline->timeExpired){return 0;}
    }

    // search wall placements
    currentMove.moveType = HORIZONTAL;
    for (int8_t i = 0; i < 64; i++){
        if (canPlaceWall(board,i,true)){
            currentMove.b1 = i;
            tryMove(board,&currentMove,depth,&alpha,&beta,deadline);
            if (alpha >= beta){return alpha;}
            if (deadline->timeExpired){return 0;}
        }
    }

    currentMove.moveType = VERTICAL;
    for (int8_t i = 0; i < 64; i++){
        if (canPlaceWall(board,i,false)){
            currentMove.b1 = i;
            tryMove(board,&currentMove,depth,&alpha,&beta,deadline);
            if (alpha >= beta){return alpha;}
            if (deadline->timeExpired){return 0;}
        }
    }

    return alpha;
}

void insertionSort(ScoredMove *scoredMoves, size_t movesCount)
{
    for (size_t i = 1; i < movesCount; i++) {
        ScoredMove current = scoredMoves[i];

        size_t j = i;

        while (j > 0 && scoredMoves[j - 1].score < current.score) {
            scoredMoves[j] = scoredMoves[j - 1];
            j--;
        }

        scoredMoves[j] = current;
    }
}

void doIteration(pBoard board,int depth,ScoredMove *legalMoves,size_t movesCount, Deadline *deadline){

    // loop over the legal moves and update their evaluation
    int alpha = -BIGGER_INF;
    for (size_t i = 0; i < movesCount; i++){
        Move currentMove = legalMoves[i].move;
        makeMove(board,&currentMove);
        int score = -negamax(board,depth - 1,-BIGGER_INF,-alpha,deadline);
        if (score > alpha){alpha = score;}
        unmakeMove(board,&currentMove);

        legalMoves[i].score = score;

        if (deadline->timeExpired){
            return;
        }
    }

    // sort the moves array based on score
    insertionSort(legalMoves,movesCount);
}

void getBestMove(pBoard board,Move *move){


    Move legalMoves[200];
    size_t legalMovesLength = getLegalMoves(board,legalMoves);
    ScoredMove scoredMoves[200];
    for (size_t i = 0; i < legalMovesLength; i++){
        ScoredMove scoredMove = {legalMoves[i],0};
        scoredMoves[i] = scoredMove;
    }


    clock_t start = clock();

    Deadline deadline = {
        start + (clock_t)(THINKING_TIME_LIMIT * CLOCKS_PER_SEC),
        0,
        false
    };

    
    int score,i;
    for (i = 2; !deadline.timeExpired; i++){
        //score = negamax(board,i,move,-BIGGER_INF,BIGGER_INF,&deadline);
        doIteration(board,i,scoredMoves,legalMovesLength,&deadline);

        if (!deadline.timeExpired){
            *move = scoredMoves[0].move;
            score = scoredMoves[0].score;
        }
    }

    printf("evaluation: %d\n",score);
    printf("thinking depth: %d\n",i-2);

}
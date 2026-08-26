#define INF 100000
#define BIGGER_INF ((INF) * 2)

#include "bot.h"
#include "assert.h"

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

int negamax(pBot bot, int depth,int alpha,int beta, Deadline *deadline);

void tryMove(pBot bot,Move *move,int depth,int *alpha,int *beta,Deadline *deadline){
    
    uint64_t originalHash = bot->boardHash;

    updateHash(bot,move);
    makeMove(bot->board,move);
    int score = -negamax(bot,depth-1,-*beta,-*alpha,deadline);
    unmakeMove(bot->board,move);
    updateHash(bot,move);

    assert(originalHash == bot->boardHash);

    if (score > *alpha){
        *alpha = score;
    }
}

int negamax(pBot bot, int depth,int alpha,int beta, Deadline *deadline){

    pBoard board = bot->board;

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
        tryMove(bot,&currentMove,depth,&alpha,&beta,deadline);
        if (alpha >= beta){return alpha;}
        if (deadline->timeExpired){return 0;}
    }

    // search wall placements
    currentMove.moveType = HORIZONTAL;
    for (int8_t i = 0; i < 64; i++){
        if (canPlaceWall(board,i,true)){
            currentMove.b1 = i;
            tryMove(bot,&currentMove,depth,&alpha,&beta,deadline);
            if (alpha >= beta){return alpha;}
            if (deadline->timeExpired){return 0;}
        }
    }

    currentMove.moveType = VERTICAL;
    for (int8_t i = 0; i < 64; i++){
        if (canPlaceWall(board,i,false)){
            currentMove.b1 = i;
            tryMove(bot,&currentMove,depth,&alpha,&beta,deadline);
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

void doIteration(pBot bot,int depth,ScoredMove *legalMoves,size_t movesCount, Deadline *deadline){

    // loop over the legal moves and update their evaluation
    int alpha = -BIGGER_INF;
    for (size_t i = 0; i < movesCount; i++){
        Move currentMove = legalMoves[i].move;

        uint64_t originalHash = bot->boardHash;

        updateHash(bot,&currentMove);
        makeMove(bot->board,&currentMove);
        int score = -negamax(bot,depth - 1,-BIGGER_INF,-alpha,deadline);
        unmakeMove(bot->board,&currentMove);
        updateHash(bot,&currentMove);

        assert(originalHash == bot->boardHash);

        if (deadline->timeExpired){
            return;
        }

        if (score > alpha){alpha = score;}
        legalMoves[i].score = score;
    }

    // sort the moves array based on score
    insertionSort(legalMoves,movesCount);
}

void getBestMove(pBot bot,Move *move){

    calculateBoardHash(bot); // calculate the hash for the current position

    pBoard board = bot->board;

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
        doIteration(bot,i,scoredMoves,legalMovesLength,&deadline);

        if (!deadline.timeExpired){
            *move = scoredMoves[0].move;
            score = scoredMoves[0].score;
        }
    }

    // printf("evaluation: %d\n",score);
    // printf("thinking depth: %d\n",i-2);

}

uint64_t splitmix64(uint64_t *state)
{
    uint64_t z = (*state += UINT64_C(0x9e3779b97f4a7c15));
    z = (z ^ (z >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94d049bb133111eb);
    return z ^ (z >> 31);
}

void updateHash(pBot bot, Move *move){
    pBoard board = bot->board;
    int player = board->turn - 1;

    bot->boardHash ^= bot->zobristValues->turnHash[0] ^ bot->zobristValues->turnHash[1];

    if (move->moveType == MOVEMENT){
        uint64_t *values = bot->zobristValues->playerHash[player];
        bot->boardHash ^= values[move->b1];
        bot->boardHash ^= values[move->b2];
    } else if (move->moveType == HORIZONTAL || move->moveType == VERTICAL){
        int index = move->moveType == HORIZONTAL ? HORIZONTAL_ZOBRIST_INDEX : VERTICAL_ZOBRIST_INDEX;
        bot->boardHash ^= bot->zobristValues->wallHash[index][move->b1];

        int wc = board->turn == 1 ? board->p1wc : board->p2wc;
        bot->boardHash ^= bot->zobristValues->wallCountHash[player][wc];
        bot->boardHash ^= bot->zobristValues->wallCountHash[player][wc - 1];

    }
}

void calculateBoardHash(pBot bot){
    // calculate the hash for the current board
    pBoard board = bot->board;
    ZobristValues *values = bot->zobristValues;

    bot->boardHash = 0;
    for (size_t i = 0; i < 64; i++){
        uint64_t mask = UINT64_C(1) << i;
        if (board->hWalls & mask){
            bot->boardHash ^= values->wallHash[HORIZONTAL_ZOBRIST_INDEX][i];
        }
        if (board->vWalls & mask){
            bot->boardHash ^= values->wallHash[VERTICAL_ZOBRIST_INDEX][i];
        }
    }
    
    bot->boardHash ^= values->playerHash[0][board->p1pos];
    bot->boardHash ^= values->playerHash[1][board->p2pos];
    bot->boardHash ^= values->wallCountHash[0][board->p1wc];
    bot->boardHash ^= values->wallCountHash[1][board->p2wc];
    bot->boardHash ^= values->turnHash[board->turn - 1];
}

pBot createBot(pBoard board){
    pBot bot = (pBot) malloc(sizeof(Bot));
    ZobristValues *values = (ZobristValues *) malloc(sizeof(ZobristValues));
    if (bot == NULL || values == NULL){
        if (bot != NULL){free(bot);}
        if (values != NULL){free(values);}
        printf("malloc failed\n");
        exit(1);
    }

    bot->board = board;

    // generate zobrist hashes
    uint64_t seed = 123456789;
    for (size_t i = 0; i < 64; i++){
        values->wallHash[HORIZONTAL_ZOBRIST_INDEX][i] = splitmix64(&seed);
        values->wallHash[VERTICAL_ZOBRIST_INDEX][i] = splitmix64(&seed);
    }

    for (int player = 0; player < 2; player++){
        for (size_t i = 0; i < 81; i++){
            values->playerHash[player][i] = splitmix64(&seed);
        }

        for (size_t i = 0; i < 11; i++){
            values->wallCountHash[player][i] = splitmix64(&seed);
        }

        values->turnHash[player] = splitmix64(&seed);
    }

    bot->zobristValues = values;

    return bot;
}

void destroyBot(pBot bot){
    free(bot->zobristValues);
    free(bot);
}
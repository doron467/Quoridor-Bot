#define INF 100000
#define BIGGER_INF ((INF) * 2)

#include "bot.h"
#include "assert.h"

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>

int evaluate(pBoard board){

    //printf("evaluate called\n");

    if (isGameOver(board)){
        // since the player just moved to the end goal, it is now the other player's turn
        // so return negative infinity to show he lost
        return -INF;
    }

    int d1 = board->pathInfo->d1;
    int d2 = board->pathInfo->d2;
    if (!board->pathInfo->updated){
        board->bfsCalls += 2;
        d1 = bfs(board->hWalls,board->vWalls,board->p1pos,0,NULL);
        d2 = bfs(board->hWalls,board->vWalls,board->p2pos,8,NULL);
    }
    
    int difference = (board->turn == 1) ? (d2 - d1) : (d1 - d2);
    return difference + 1;
}

int negamax(pBot bot, int depth,int alpha,int beta, Deadline *deadline);

void tryMove(pBot bot,Move *move,Move *bestMove,int *bestScore,int depth,int *alpha,int *beta,Deadline *deadline){
    
    //uint64_t originalHash = bot->boardHash;
    if (move->moveType == MOVEMENT){
        move->b1 = bot->board->turn == 1 ? bot->board->p1pos : bot->board->p2pos;
    }

    updateHash(bot,move);
    makeMove(bot->board,move);

    int score;
    if (bestMove->moveType == NULL_MOVE){
        score = -negamax(bot,depth-1,-*beta,-*alpha,deadline);
    } else {
        // do a null search first
        score = -negamax(bot,depth-1,-*alpha - 1,-*alpha,deadline);

        // debug stuff
        if (score <= *alpha){
            bot->pvsFailLow++;
        } else if (score >= *beta){
            bot->pvsFailHigh++;
        }

        // if (*beta > *alpha + 1){
        //     bot->wideWindows++;
        // }

        if (score > *alpha && score < *beta){
            // score was good enough to beat the last candidate, but not good enough to beat beta
            // need to perform a full search on it
            bot->pvsResearches++;
            score = -negamax(bot,depth-1,-*beta,-*alpha,deadline);
        }
    }

    unmakeMove(bot->board,move);
    updateHash(bot,move);

    //assert(originalHash == bot->boardHash);

    if (score > *bestScore){
        *bestScore = score;
        *bestMove = *move;
    }

    if (score > *alpha){
        *alpha = score;
    }
}

int negamax(pBot bot, int depth,int alpha,int beta, Deadline *deadline){

    pBoard board = bot->board;
    int originalAlpha = alpha;
    int originalBeta = beta;

    if ((deadline->nodesCount++) == CLOCK_CHECK){
        deadline->nodesCount = 0;
        if (clock() > deadline->deadline){
            deadline->timeExpired = true;
            return 0;
        }
    }
    
    // tt lookup
    TTEntry *entry = &bot->transpositionsTable[bot->boardHash & (TT_SIZE - 1)];
    Move ttMove = {.moveType = NULL_MOVE};
    Move bestMove = {.moveType = NULL_MOVE};
    int bestScore = -BIGGER_INF;

    // debugging
    bot->nodesVisited++;
    if (entry->key == bot->boardHash && entry->depth != -1){
        bot->ttMatches++;
    }
    if (entry->key == bot->boardHash && entry->depth >= depth){
        bot->ttHits++;
    }
    if (entry->depth != -1 && entry->key != bot->boardHash){
        bot->ttCollisions++;
    }
    if (beta - alpha > 1){
        bot->wideWindows++;
    }

    if (entry->key == bot->boardHash && entry->depth >= depth) {

        if (entry->flag == TT_EXACT){
            return entry->score;
        }

        if (entry->flag == TT_LOWER_BOUND) {
            if (entry->score >= beta){
                return entry->score;
            }
            if (entry->score > alpha){
                alpha = entry->score;
            }
        } else if (entry->flag == TT_UPPER_BOUND){
            if (entry->score < alpha){
                return entry->score;
            }
            if (entry->score < beta){
                beta = entry->score;
            }
        }

        if (alpha >= beta){
            return alpha;
        }

    }

    if (depth == 0 || isGameOver(board)){
        return evaluate(board);
    }

    PathInfo pathInfoCopy;
    uint32_t path1[TILES_HASHSET_LENGTH];
    uint32_t path2[TILES_HASHSET_LENGTH];
    copyPathInfo(bot->board->pathInfo,&pathInfoCopy,path1,path2);
    board->pathInfo = &pathInfoCopy;

    // try the transpositions table best move first
    if (entry->key == bot->boardHash && entry->depth != -1){
        ttMove = entry->bestMove;
        tryMove(bot,&ttMove,&bestMove,&bestScore,depth,&alpha,&beta,deadline);
        if (deadline->timeExpired){return 0;}
    }


    int8_t movementBuffer[10];
    getPlayerMoves(board,movementBuffer);

    // try killer moves next
    int ply = bot->currentDepth - depth;
    Move killer0 = bot->killerMoves[ply][0];
    Move killer1 = bot->killerMoves[ply][1];
    board->pathInfo = &pathInfoCopy;
    if (alpha < beta && isLegalMove(board,&killer0,movementBuffer)){
        bot->killerAttempts++;
        tryMove(bot,&killer0,&bestMove,&bestScore,depth,&alpha,&beta,deadline);
        if (deadline->timeExpired){return 0;}

        if (alpha >= beta){
            bot->killerCutoffs++;
        }
    }
    board->pathInfo = &pathInfoCopy;
    if (alpha < beta && isLegalMove(board,&killer1,movementBuffer)){
        bot->killerAttempts++;
        tryMove(bot,&killer1,&bestMove,&bestScore,depth,&alpha,&beta,deadline);
        if (deadline->timeExpired){return 0;}

        if (alpha >= beta){
            bot->killerCutoffs++;
        }
    }

    
    Move currentMove;


    // search movements
    currentMove.moveType = MOVEMENT;
    currentMove.b1 = (board->turn == 1) ? board->p1pos : board->p2pos;
    for (int i = 0; movementBuffer[i] != -1 && alpha < beta; i++){
        currentMove.b2 = movementBuffer[i];
        if (sameMove(&currentMove,&ttMove)){continue;}
        if (sameMove(&currentMove,&killer0)){continue;}
        if (sameMove(&currentMove,&killer1)){continue;}

        board->pathInfo = &pathInfoCopy;
        tryMove(bot,&currentMove,&bestMove,&bestScore,depth,&alpha,&beta,deadline);

        if (alpha >= beta){
            bot->killerMoves[ply][1] = bot->killerMoves[ply][0];
            bot->killerMoves[ply][0] = currentMove;
        }

        if (deadline->timeExpired){return 0;}
    }

    // search wall placements
    currentMove.moveType = HORIZONTAL;
    for (int8_t i = 0; i < 64 && alpha < beta; i++){
        currentMove.b1 = i;
        if (sameMove(&currentMove,&ttMove)){continue;}
        if (sameMove(&currentMove,&killer0)){continue;}
        if (sameMove(&currentMove,&killer1)){continue;}

        board->pathInfo = &pathInfoCopy;
        if (canPlaceWall(board,i,true)){
            tryMove(bot,&currentMove,&bestMove,&bestScore,depth,&alpha,&beta,deadline);

            if (alpha >= beta){
                bot->killerMoves[ply][1] = bot->killerMoves[ply][0];
                bot->killerMoves[ply][0] = currentMove;
            }

            if (deadline->timeExpired){return 0;}
        }
    }

    currentMove.moveType = VERTICAL;
    for (int8_t i = 0; i < 64 && alpha < beta; i++){
        currentMove.b1 = i;
        if (sameMove(&currentMove,&ttMove)){continue;}
        if (sameMove(&currentMove,&killer0)){continue;}
        if (sameMove(&currentMove,&killer1)){continue;}

        board->pathInfo = &pathInfoCopy;
        if (canPlaceWall(board,i,false)){
            tryMove(bot,&currentMove,&bestMove,&bestScore,depth,&alpha,&beta,deadline);
            //if (alpha >= beta){return alpha;}

            if (alpha >= beta){
                bot->killerMoves[ply][1] = bot->killerMoves[ply][0];
                bot->killerMoves[ply][0] = currentMove;
            }

            if (deadline->timeExpired){return 0;}
        }
    }

    TTFlag flag;
    if (alpha <= originalAlpha) {
        // no move better than alpha was found, making it an upper bound
        flag = TT_UPPER_BOUND;
    }
    else if (alpha >= originalBeta) {
        // a move better than beta was found, so the search stopped midway, making alpha a lower bound
        flag = TT_LOWER_BOUND;
    }
    else {
        // if there was no cutoff, then a full search happened, giving us an exact result
        flag = TT_EXACT;
    }

    if (entry->depth <= depth){
        entry->key = bot->boardHash;
        entry->flag = flag;
        entry->depth = depth;
        entry->score = alpha;
        entry->bestMove = bestMove;
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
    bot->currentDepth = depth;
    int alpha = -BIGGER_INF;
    PathInfo *originalPathInfo = bot->board->pathInfo;
    printf("depth: %d\n",depth);
    for (size_t i = 0; i < movesCount; i++){
        Move currentMove = legalMoves[i].move;
        //printf("evaluating move: ");
        //printMove(&currentMove);

        //uint64_t originalHash = bot->boardHash;

        bot->board->pathInfo = originalPathInfo; // reset path info after the last negamax search
        if (currentMove.moveType == HORIZONTAL || currentMove.moveType == VERTICAL){
            // this function is called to set the board path info parameters for negamax
            canPlaceWall(bot->board,currentMove.b1,currentMove.moveType == HORIZONTAL);
        }

        updateHash(bot,&currentMove);
        makeMove(bot->board,&currentMove);
        //int score = -negamax(bot,depth - 1,-BIGGER_INF,-alpha,deadline);

        int score;
        if (i == 0){
            score = -negamax(bot,depth-1,-BIGGER_INF,-alpha,deadline);
        } else {
            // do a null search first, only do a full if necessary
            score = -negamax(bot,depth-1,-alpha - 1,-alpha,deadline);
            if (score > alpha){
                score = -negamax(bot,depth-1,-BIGGER_INF,-alpha,deadline);
            }
        }

        unmakeMove(bot->board,&currentMove);
        updateHash(bot,&currentMove);

        //assert(originalHash == bot->boardHash);

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

    // debugging info
    bot->killerAttempts = 0;
    bot->killerCutoffs = 0;
    bot->ttCollisions = 0;
    bot->ttHits = 0;
    bot->nodesVisited = 0;
    bot->ttMatches = 0;
    bot->pvsResearches = 0;
    bot->pvsFailHigh = 0;
    bot->pvsFailLow = 0;
    bot->wideWindows = 0;
    bot->board->bfsCalls = 0;

    pBoard board = bot->board;
    PathInfo pathInfo;
    uint32_t path1[TILES_HASHSET_LENGTH];
    uint32_t path2[TILES_HASHSET_LENGTH];
    pathInfo.updated = false;
    pathInfo.path1 = path1;
    pathInfo.path2 = path2;
    board->pathInfo = &pathInfo;

    // reset killer moves to null
    Move nullMove = {.moveType = NULL_MOVE};
    for (size_t i = 0; i < MAX_DEPTH; i++){
        bot->killerMoves[i][0] = nullMove;
        bot->killerMoves[i][1] = nullMove;
    }

    // get all legal root moves
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
    for (i = 2; !deadline.timeExpired && i <= MAX_DEPTH; i++){
        //score = negamax(board,i,move,-BIGGER_INF,BIGGER_INF,&deadline);
        board->pathInfo = &pathInfo;
        doIteration(bot,i,scoredMoves,legalMovesLength,&deadline);

        if (!deadline.timeExpired){
            *move = scoredMoves[0].move;
            score = scoredMoves[0].score;
        }
    }

    double time = (clock() - start) / (double) CLOCKS_PER_SEC;

    printf("nodes visited: %ld\nhits: %ld\ncollisions: %ld\n",bot->nodesVisited,bot->ttHits,bot->ttCollisions);


    // hit ratio drops when depth is odd
    // the reason for that is a mistake in the calculation, caused by including the leafs of the tree
    printf("hit ratio: %f%%\n",bot->ttHits / (double) bot->nodesVisited * 100);

    printf("match ratio: %f%%\n",bot->ttHits / (double) bot->ttMatches * 100);
    printf("collision ratio: %f%%\n",bot->ttCollisions / (double) bot->nodesVisited * 100);
    printf("bfs calls: %lu\n",board->bfsCalls);

    printf("pvs fail low: %ld\n",bot->pvsFailLow);
    printf("pvs fail high: %ld\n",bot->pvsFailHigh);
    printf("pvs researches: %ld\n",bot->pvsResearches);
    printf("wide windows: %ld\n",bot->wideWindows);

    printf("killer cutoff ratio: %f%%\n",bot->killerCutoffs / (double) bot->killerAttempts * 100);


    printf("evaluation: %d\n",score);
    printf("thinking depth: %d\n",i - (deadline.timeExpired ? 2 : 1));
    printf("thinking time: %f\n",time);

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

    // initialize transpositions table
    TTEntry emptyEntry = {.depth = -1};
    for (uint64_t i = 0; i < TT_SIZE; i++){
        bot->transpositionsTable[i] = emptyEntry;
    }

    return bot;
}

void destroyBot(pBot bot){
    free(bot->zobristValues);
    free(bot);
}
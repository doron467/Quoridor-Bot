#include "bot.h"
#include <stdio.h>

void getBestMove(pBoard board,char *buffer){
    uint8_t turn = board->turn;
    int8_t moves[10];
    getPlayerMoves(board,moves);

    Board temp = *board;
    if (turn == 1){temp.p1pos = moves[0];} else {temp.p2pos = moves[0];}
    int bestEval = bfs(temp.hWalls,temp.vWalls,temp.p1pos,0) - bfs(temp.hWalls,temp.vWalls,temp.p2pos,8);
    int8_t bestMoves[10] = {moves[0]};
    uint8_t bestHorizontals[100];
    uint8_t bestVerticals[100];
    int movesLength = 1;
    int horizontalsLength = 0;
    int verticalsLength = 0;

    for (int i = 1; moves[i] != -1; i++){
        int8_t move = moves[i];
        printf("evaluating move %d\n",move);
        temp = *board;
        if (turn == 1){temp.p1pos = move;} else {temp.p2pos = move;}
        int eval = bfs(temp.hWalls,temp.vWalls,temp.p1pos,0) - bfs(temp.hWalls,temp.vWalls,temp.p2pos,8);
        if ((turn == 1 && eval < bestEval) || (turn == 2 && eval > bestEval)){
            bestEval = eval;
            bestMoves[0] = move;
            movesLength = 1;
        } else if (eval == bestEval){
            bestMoves[movesLength++] = move;
        }
    }


    for (int i = 0; i < 81; i++){
        temp = *board;
        if (canPlaceWall(&temp,i,true)){
            int eval = bfs(temp.hWalls,temp.vWalls,temp.p1pos,0) - bfs(temp.hWalls,temp.vWalls,temp.p2pos,8);
            if ((turn == 1 && eval < bestEval) || (turn == 2 && eval > bestEval)){
                bestEval = eval;
                bestHorizontals[0] = i;
                horizontalsLength = 1;
                movesLength = 0;
            } else if (eval == bestEval){
                bestHorizontals[horizontalsLength++] = i;
            }
        }
    }

    for (int i = 0; i < 81; i++){
        temp = *board;
        if (canPlaceWall(&temp,i,false)){
            int eval = bfs(temp.hWalls,temp.vWalls,temp.p1pos,0) - bfs(temp.hWalls,temp.vWalls,temp.p2pos,8);
            if ((turn == 1 && eval < bestEval) || (turn == 2 && eval > bestEval)){
                bestEval = eval;
                bestVerticals[0] = i;
                verticalsLength = 1;
                horizontalsLength = 0;
                movesLength = 0;
            } else if (eval == bestEval){
                bestVerticals[verticalsLength++] = i;
            }
        }
    }
    
    printf("%d %d %d\n",movesLength,horizontalsLength,verticalsLength);
    printf("best eval: %d\n",bestEval);

    // decide randomly between moving and placing a wall
    bool movement = false;
    bool placement = false;
    if (movesLength > 0 && (horizontalsLength > 0 || verticalsLength > 0)){
        if (rand() % 2 && rand() % 2){
            placement = true;
        } else {
            movement = true;
        }
    }

    if ((horizontalsLength == 0 && verticalsLength == 0) || movement){
        int8_t move = bestMoves[0];
        printf("tile number: %d\n",move);
        buffer[0] = 'm';
        buffer[1] = (char) (move / 9 + '0');
        buffer[2] = (char) (move % 9 + '0');
        buffer[3] = '\0';
        return;
    }

    if (movesLength == 0 || placement){

        bool horizontal = false;
        bool vertical = false;
        if (horizontalsLength > 0 && verticalsLength > 0){
            if (rand() % 2){
                horizontal = true;
            } else {
                vertical = true;
            }
        }

        uint8_t wall;
        if (horizontalsLength == 0 || vertical){
            buffer[0] = 'v';
            wall = bestVerticals[0];
        } else if (verticalsLength == 0 || horizontal){
            buffer[0] = 'h';
            wall = bestHorizontals[0];
        }

        buffer[1] = (char) (wall / 8 + '0');
        buffer[2] = (char) (wall % 8 + '0');
        buffer[3] = '\0';
        return;
    }

}
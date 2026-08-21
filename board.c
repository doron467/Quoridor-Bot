#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
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
    for (int row = 0; row < 9; row++){
        // print row
        for (int column = 0; column < 9; column++){
            char c = '*';
            if (board->p1pos == row * 9 + column){c = '1';}
            if (board->p2pos == row * 9 + column){c = '2';}
            
            char left = ' ';
            if (column > 0){
                if (row > 0){
                    if (board->vWalls & (UINT64_C(1) << ((row - 1) * 8 + column - 1))){
                        left = '|';
                    }
                }
                if (row < 8){
                    if (board->vWalls & (UINT64_C(1) << (row * 8 + column - 1))){
                        left = '|';
                    }
                }
            }

            char right = ' ';
            if (column < 8){
                if (row > 0){
                    if (board->vWalls & (UINT64_C(1) << ((row - 1) * 8 + column))){
                        right = '|';
                    }
                }
                if (row < 8){
                    if (board->vWalls & (UINT64_C(1) << (row * 8 + column))){
                        right = '|';
                    }
                }
            }

            buffer[j++] = left;
            buffer[j++] = c;
            buffer[j++] = right;
            
        }

        buffer[j++] = '\n';
        // print horizontal walls
        if (row == 8){break;}
        for (int column = 0; column < 9; column++){
            char c = ' ';
            if (column > 0){
                if (board->hWalls & (UINT64_C(1) << (row * 8 + column - 1))){
                    c = '-';
                }
            }
            if (column < 8){
                if (board->hWalls & (UINT64_C(1) << (row * 8 + column))){
                    c = '-';
                }
            }
            buffer[j++] = c;
            buffer[j++] = c;
            buffer[j++] = c;
        }
        buffer[j++] = '\n';
    }
    buffer[j] = '\0';

    printf("%s\n",buffer);
    printf("player1 walls count: %d\n",board->p1wc);
    printf("player2 walls count: %d\n",board->p2wc);
    printf("turn: p%d\n",board->turn);
}

bool isQueued(int8_t pos, const uint32_t *queued)
{
    if (pos < 32) {
        return (queued[0] >> pos) & 1;
    } else if (pos < 64) {
        return (queued[1] >> (pos - 32)) & 1;
    } else {
        return (queued[2] >> (pos - 64)) & 1;
    }
}

void addToQueued(int8_t pos, uint32_t *queued)
{
    if (pos < 32) {
        queued[0] |= UINT32_C(1) << pos;
    } else if (pos < 64) {
        queued[1] |= UINT32_C(1) << (pos - 32);
    } else {
        queued[2] |= UINT32_C(1) << (pos - 64);
    }
}

int bfs(uint64_t hWalls,uint64_t vWalls,int8_t start,int rankTarget){
    uint8_t queue[BOARD_SIZE];
    uint32_t queued[3] = {0};

    queue[0] = start;
    addToQueued(start,queued);

    size_t head = 0;
    size_t tail = 1;
    size_t depth = 0;

    while (head < tail){

        int nodesInLevel = tail - head;

        for (size_t i = 0; i < nodesInLevel; i++){
            int8_t pos = queue[head++];
            if (pos >= rankTarget * 9 && pos < (rankTarget + 1) * 9){
                return depth;
            }

            int row = pos / 9;
            int column = pos % 9;

            if (CAN_LEFT(pos) && !isQueued(LEFT(pos),queued)){
                int8_t w1 = row * 8 + column - 1;
                int8_t w2 = w1 - 8;
                uint64_t mask = 0;
                if (w1 < 64){
                    mask |= UINT64_C(1) << w1;
                }
                if (w2 >= 0){
                    mask |= UINT64_C(1) << w2;
                }
                if ((vWalls & mask) == 0){
                    addToQueued(LEFT(pos),queued);
                    queue[tail++] = LEFT(pos);
                }
            }

            if (CAN_RIGHT(pos) && !isQueued(RIGHT(pos),queued)){
                int8_t w1 = row * 8 + column;
                int8_t w2 = w1 - 8;
                uint64_t mask = 0;
                if (w1 < 64){
                    mask |= UINT64_C(1) << w1;
                }
                if (w2 >= 0){
                    mask |= UINT64_C(1) << w2;
                }
                if ((vWalls & mask) == 0){
                    addToQueued(RIGHT(pos),queued);
                    queue[tail++] = RIGHT(pos);
                }
            }

            if (CAN_UP(pos) && !isQueued(UP(pos),queued)){
                int8_t w1 = (row - 1) * 8 + column;
                int8_t w2 = w1 - 1;
                uint64_t mask = 0;
                if (w1 < 64){
                    mask |= UINT64_C(1) << w1;
                }
                if (w2 >= 0){
                    mask |= UINT64_C(1) << w2;
                }
                if ((hWalls & mask) == 0){
                    addToQueued(UP(pos),queued);
                    queue[tail++] = UP(pos);
                }
            }

            if (CAN_DOWN(pos) && !isQueued(DOWN(pos),queued)){
                int8_t w1 = row * 8 + column;
                int8_t w2 = w1 - 1;
                uint64_t mask = 0;
                if (w1 < 64){
                    mask |= UINT64_C(1) << w1;
                }
                if (w2 >= 0){
                    mask |= UINT64_C(1) << w2;
                }
                if ((hWalls & mask) == 0){
                    addToQueued(DOWN(pos),queued);
                    queue[tail++] = DOWN(pos);
                }
            }
        }

        depth++;
        
    }

    return -1;
}

bool placeWall(pBoard board,int8_t position,bool horizontal){

    if ((board->turn == 1 ? board->p1wc : board->p2wc) <= 0){
        return false; // out of walls to place
    }

    if (position < 0 || position >= 64){
        return false; // position out of bounds
    }

    uint64_t mask = UINT64_C(1) << position;
    uint64_t hUpdated = board->hWalls;
    uint64_t vUpdated = board->vWalls;

    if ((board->hWalls & mask) || (board->vWalls & mask)){
        return false; // walls cannot cross
    }

    if (horizontal){
        hUpdated |= mask;

        if ((position % 8 != 0) && (board->hWalls & (mask >> 1))){
            return false; // wall already placed to the left
        }

        if ((position % 8 != 7) && (board->hWalls & (mask << 1))){
            return false; // wall already placed to the right
        }

    } else {
        vUpdated |= mask;

        if ((position - 8 >= 0) && (board->vWalls & (mask >> 8))){
            return false; // wall already placed above
        }

        if ((position + 8 < 64) && (board->vWalls & (mask << 8))){
            return false; // wall already placed below
        }
    }
    
    int depth1 = bfs(hUpdated,vUpdated,board->p1pos,0);
    if (depth1 == -1){
        return false; // can't trap player 1
    }

    int depth2 = bfs(hUpdated,vUpdated,board->p2pos,8);
    if (depth2 == -1){
        return false; // can't trap player 2
    }

    // place the wall in the real board
    if (horizontal){board->hWalls |= mask;} else {board->vWalls |= mask;}
    return true;
}

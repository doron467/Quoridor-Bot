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

    buffer[j++] = ' ';
    for (int column = 0; column < 9; column++){
        buffer[j++] = ' ';
        buffer[j++] = (char) (column + '0');
        buffer[j++] = ' ';
    }
    buffer[j++] = '\n';

    for (int row = 0; row < 9; row++){
        // print row
        buffer[j++] = (char) (row + '0');
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
        buffer[j++] = ' ';
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

int8_t directionInBounds(int8_t pos, Direction dir){
    switch (dir){
        case LEFT:
            if (CAN_LEFT(pos)){
                return MOVE_LEFT(pos);
            }
            break;

        case RIGHT:
            if (CAN_RIGHT(pos)){
                return MOVE_RIGHT(pos);
            }
            break;

        case UP:
            if (CAN_UP(pos)){
                return MOVE_UP(pos);
            }
            break;

        case DOWN:
            if (CAN_DOWN(pos)){
                return MOVE_DOWN(pos);
            }
            break;
    }
    return -1;
}

bool hasWall(uint64_t walls,int row,int column){
    return walls & (UINT64_C(1) << row * 8 + column);
}

bool isBlocked(uint64_t hWalls,uint64_t vWalls,int8_t startPos,int row,int column, Direction dir){
    if (directionInBounds(startPos,dir) != -1){
        
        if (dir == LEFT){
            if (column > 0){
                if (hasWall(vWalls,row-1,column-1)){
                    return true;
                }
            }
            if (column < 8){
                if (hasWall(vWalls,row,column-1)){
                    return true;
                }
            }
        } else if (dir == RIGHT){
            if (column > 0){
                if (hasWall(vWalls,row-1,column)){
                    return true;
                }
            }
            if (column < 8){
                if (hasWall(vWalls,row,column)){
                    return true;
                }
            }
        } else if (dir == UP){
            if (row > 0){
                if (hasWall(hWalls,row-1,column-1)){
                    return true;
                }
            }
            if (row < 8){
                if (hasWall(hWalls,row-1,column)){
                    return true;
                }
            }
        } else if (dir == DOWN){
            if (row > 0){
                if (hasWall(hWalls,row,column-1)){
                    return true;
                }
            }
            if (row < 8){
                if (hasWall(hWalls,row,column)){
                    return true;
                }
            }
        }

    } else {
        return true;
    }

    return false;
}

void getPlayerMoves(pBoard board,int8_t *buffer){
    int8_t moverPos, otherPos;
    if (board->turn == 1){
        moverPos = board->p1pos;
        otherPos = board->p2pos;
    } else {
        moverPos = board->p2pos;
        otherPos = board->p1pos;
    }

    int row = moverPos / 9;
    int column = moverPos % 9;
    uint64_t hWalls = board->hWalls;
    uint64_t vWalls = board->vWalls;

    if (!isBlocked(hWalls,vWalls,moverPos,row,column,LEFT)){
        if (MOVE_LEFT(moverPos) == otherPos){
            if (!isBlocked(hWalls,vWalls,otherPos,row,column-1,LEFT)){
                *(buffer++) = MOVE_LEFT(otherPos);
            } else {
                if (!isBlocked(hWalls,vWalls,otherPos,row,column-1,UP)){
                    *(buffer++) = MOVE_UP(otherPos);
                }
                if (!isBlocked(hWalls,vWalls,otherPos,row,column-1,DOWN)){
                    *(buffer++) = MOVE_DOWN(otherPos);
                }
            }
        } else {
            *(buffer++) = MOVE_LEFT(moverPos);
        }
    }


    if (!isBlocked(hWalls,vWalls,moverPos,row,column,RIGHT)){
        if (MOVE_RIGHT(moverPos) == otherPos){
            if (!isBlocked(hWalls,vWalls,otherPos,row,column+1,RIGHT)){
                *(buffer++) = MOVE_RIGHT(otherPos);
            } else {
                if (!isBlocked(hWalls,vWalls,otherPos,row,column+1,UP)){
                    *(buffer++) = MOVE_UP(otherPos);
                }
                if (!isBlocked(hWalls,vWalls,otherPos,row,column+1,DOWN)){
                    *(buffer++) = MOVE_DOWN(otherPos);
                }
            }
        } else {
            *(buffer++) = MOVE_RIGHT(moverPos);
        }
    }


    if (!isBlocked(hWalls,vWalls,moverPos,row,column,UP)){
        if (MOVE_UP(moverPos) == otherPos){
            if (!isBlocked(hWalls,vWalls,otherPos,row-1,column,UP)){
                *(buffer++) = MOVE_UP(otherPos);
            } else {
                if (!isBlocked(hWalls,vWalls,otherPos,row-1,column,LEFT)){
                    *(buffer++) = MOVE_LEFT(otherPos);
                }
                if (!isBlocked(hWalls,vWalls,otherPos,row-1,column,RIGHT)){
                    *(buffer++) = MOVE_RIGHT(otherPos);
                }
            }
        } else {
            *(buffer++) = MOVE_UP(moverPos);
        }
    }


    if (!isBlocked(hWalls,vWalls,moverPos,row,column,DOWN)){
        if (MOVE_DOWN(moverPos) == otherPos){
            if (!isBlocked(hWalls,vWalls,otherPos,row+1,column,DOWN)){
                *(buffer++) = MOVE_DOWN(otherPos);
            } else {
                if (!isBlocked(hWalls,vWalls,otherPos,row+1,column,LEFT)){
                    *(buffer++) = MOVE_LEFT(otherPos);
                }
                if (!isBlocked(hWalls,vWalls,otherPos,row+1,column,RIGHT)){
                    *(buffer++) = MOVE_RIGHT(otherPos);
                }
            }
        } else {
            *(buffer++) = MOVE_DOWN(moverPos);
        }
    }

    *buffer = -1;
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
    int8_t parent[81];

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

                // print path
                /*
                int8_t temp = pos;
                while (temp != start){
                    printf("%d <-- ",temp);
                    temp = parent[temp];
                }
                printf("%d\n",temp);
                */
                return depth;
            }

            int row = pos / 9;
            int column = pos % 9;

            if (!isQueued(MOVE_LEFT(pos),queued)){
                if (!isBlocked(hWalls,vWalls,pos,row,column,LEFT)){
                    addToQueued(MOVE_LEFT(pos),queued);
                    queue[tail++] = MOVE_LEFT(pos);
                    parent[MOVE_LEFT(pos)] = pos;
                }
            }

            if (!isQueued(MOVE_RIGHT(pos),queued)){
                if (!isBlocked(hWalls,vWalls,pos,row,column,RIGHT)){
                    addToQueued(MOVE_RIGHT(pos),queued);
                    queue[tail++] = MOVE_RIGHT(pos);
                    parent[MOVE_RIGHT(pos)] = pos;
                }
            }

            if (!isQueued(MOVE_UP(pos),queued)){
                if (!isBlocked(hWalls,vWalls,pos,row,column,UP)){
                    addToQueued(MOVE_UP(pos),queued);
                    queue[tail++] = MOVE_UP(pos);
                    parent[MOVE_UP(pos)] = pos;
                }
            }

            if (CAN_DOWN(pos) && !isQueued(MOVE_DOWN(pos),queued)){
                if (!isBlocked(hWalls,vWalls,pos,row,column,DOWN)){
                    addToQueued(MOVE_DOWN(pos),queued);
                    queue[tail++] = MOVE_DOWN(pos);
                    parent[MOVE_DOWN(pos)] = pos;
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
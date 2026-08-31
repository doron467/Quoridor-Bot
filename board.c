#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include "board.h"
#include "bot.h"

pBoard initializeBoard(uint8_t turn){

    pBoard gameBoard = (pBoard) malloc(sizeof(Board));
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
    gameBoard->turn = turn;
    gameBoard->lastMoveIndex = 0;

    gameBoard->bfsCalls = 0;
    return gameBoard;
}

bool isGameOver(pBoard board){
    return !CAN_UP(board->p1pos) || !CAN_DOWN(board->p2pos);
}

void printMove(Move *move){
    if (move->moveType == MOVEMENT){
        int row = move->b2 / 9;
        int column = move->b2 % 9;
        printf("m%d%d\n",row,column);
    } else {
        int row = move->b1 / 8;
        int column = move->b1 % 8;
        printf("%c%d%d\n",move->moveType == HORIZONTAL ? 'h' : 'v',row,column);
    }
}

void copyPathSet(uint32_t *originalPath,uint32_t *copyPath){
    for (size_t i = 0; i < TILES_HASHSET_LENGTH; i++){
        copyPath[i] = originalPath[i];
    }
}

int manhattanDistance(int8_t pos1,int8_t pos2){
    int row1 = pos1 / 9;
    int col1 = pos1 % 9;
    int row2 = pos2 / 9;
    int col2 = pos2 % 9;

    return abs(row1 - row2) + abs(col1 - col2);
}

void printMoveHistory(pBoard board){
    for (size_t i = 0; i < board->lastMoveIndex; i++){
        printMove(&board->moveHistory[i]);
    }
}

void makeMove(pBot bot,Move *move,size_t ply){
    //board->pathInfo->updated = true;
    pBoard board = bot->board;
    board->moveHistory[board->lastMoveIndex++] = *move;

    if (move->moveType == MOVEMENT){
        int8_t startPos = move->b1;
        int8_t target = move->b2;

        // path info needs to be updated
        pPathInfo pathInfo = &(bot->pathInfos[ply]);
        pPathInfo nextPath = &(bot->pathInfos[ply + 1]);
        uint32_t *pathSet = board->turn == 1 ? pathInfo->path1 : pathInfo->path2;
        uint32_t *nextSet = board->turn == 1 ? nextPath->path1 : nextPath->path2;

        if (inSet(target,pathSet)){ // player is following his best path, no need to recalculate
            // update the hashset
            copyPathSet(pathSet,nextSet);

            removeFromSet(startPos,nextSet);
            int dist = manhattanDistance(startPos,target);
            if (dist == 2){ // is a jump
                removeNeighboursFromSet(board,startPos,nextSet);
            }

            // update the distance
            if (board->turn == 1){
                nextPath->d1 = pathInfo->d1 - dist;
                assert(nextPath->d1 ==bfs(board->hWalls, board->vWalls,target,0,NULL));
            } else {
                nextPath->d2 = pathInfo->d2 - dist;
                assert(nextPath->d2 ==bfs(board->hWalls, board->vWalls,target,8,NULL));
            }

        } else {
            // need to recalculate from scratch
            int rankTarget = board->turn == 1 ? 0 : 8;
            int d = bfs(board->hWalls,board->vWalls,target,rankTarget,nextSet);
            if (board->turn == 1){nextPath->d1 = d;}else{nextPath->d2 = d;}
        }

        // copy the other player's path from the previous depth
        if (board->turn == 1){
            copyPathSet(pathInfo->path2,nextPath->path2);
            nextPath->d2 = pathInfo->d2;
        } else {
            copyPathSet(pathInfo->path1,nextPath->path1);
            nextPath->d1 = pathInfo->d1;
        }


        if (board->turn == 1){board->p1pos = target;} else {board->p2pos = target;}
        

    } else {

        //board->pathInfo->updated = true;
        if (move->moveType == HORIZONTAL){
            board->hWalls |= UINT64_C(1) << move->b1;
        } else {
            board->vWalls |= UINT64_C(1) << move->b1;
        }

        if (board->turn == 1){board->p1wc--;} else {board->p2wc--;}

    }

    SWITCH_TURNS(board);
}

void unmakeMove(pBoard board, Move *move){

    SWITCH_TURNS(board);
    board->lastMoveIndex--;
    //board->pathInfo->updated = false;

    if (move->moveType == MOVEMENT){
        int8_t target = (int8_t) move->b1;
        if (board->turn == 1){board->p1pos = target;} else {board->p2pos = target;}
    } else {

        if (move->moveType == HORIZONTAL){
            board->hWalls ^= UINT64_C(1) << move->b1;
        } else {
            board->vWalls ^= UINT64_C(1) << move->b1;
        }

        if (board->turn == 1){board->p1wc++;} else {board->p2wc++;}

    }

}

bool sameMove(Move *move1, Move *move2){
    
    if (move1->moveType == NULL_MOVE || move2->moveType == NULL_MOVE){
        return move1->moveType == move2->moveType;
    }

    if (move1->moveType == MOVEMENT && move2->moveType == MOVEMENT){
        return move1->b2 == move2->b2;
    }
    return move1->b1 == move2->b1;
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

int8_t moveDirection(int8_t pos, Direction dir){
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
    if (moveDirection(startPos,dir) != -1){
        
        if (dir == LEFT){
            if (row > 0){
                if (hasWall(vWalls,row-1,column-1)){
                    return true;
                }
            }
            if (row < 8){
                if (hasWall(vWalls,row,column-1)){
                    return true;
                }
            }
        } else if (dir == RIGHT){
            if (row > 0){
                if (hasWall(vWalls,row-1,column)){
                    return true;
                }
            }
            if (row < 8){
                if (hasWall(vWalls,row,column)){
                    return true;
                }
            }
        } else if (dir == UP){
            if (column > 0){
                if (hasWall(hWalls,row-1,column-1)){
                    return true;
                }
            }
            if (column < 8){
                if (hasWall(hWalls,row-1,column)){
                    return true;
                }
            }
        } else if (dir == DOWN){
            if (column > 0){
                if (hasWall(hWalls,row,column-1)){
                    return true;
                }
            }
            if (column < 8){
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

size_t getPlayerMoves(pBoard board,int8_t *buffer){

    int8_t *bufferCopy = buffer;
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
    return buffer - bufferCopy;
}

bool inSet(int8_t pos, const uint32_t *hashset)
{
    if (pos < 32) {
        return (hashset[0] >> pos) & 1;
    } else if (pos < 64) {
        return (hashset[1] >> (pos - 32)) & 1;
    } else {
        return (hashset[2] >> (pos - 64)) & 1;
    }
}

void addToSet(int8_t pos, uint32_t *hashset)
{
    if (pos < 32) {
        hashset[0] |= UINT32_C(1) << pos;
    } else if (pos < 64) {
        hashset[1] |= UINT32_C(1) << (pos - 32);
    } else {
        hashset[2] |= UINT32_C(1) << (pos - 64);
    }
}

void removeFromSet(int8_t pos,uint32_t *hashset){
    if (!inSet(pos,hashset)){
        printf("warning: remove from set called on bit 0\n");
        return;
    }

    if (pos < 32) {
        hashset[0] ^= UINT32_C(1) << pos;
    } else if (pos < 64) {
        hashset[1] ^= UINT32_C(1) << (pos - 32);
    } else {
        hashset[2] ^= UINT32_C(1) << (pos - 64);
    }
}

void removeNeighboursFromSet(pBoard board,int8_t pos,uint32_t *hashset){
    Direction directions[] = {LEFT,RIGHT,UP,DOWN};
    int row = pos / 9;
    int column = pos % 9;
    for (size_t i = 0; i < 4; i++){
        Direction dir = directions[i];
        if (!isBlocked(board->hWalls,board->vWalls,pos,row,column,dir)){
            int8_t target = moveDirection(pos,dir);
            if (inSet(target,hashset)){
                removeFromSet(target,hashset);
            }
        }
    }
}

int bfs(uint64_t hWalls,uint64_t vWalls,int8_t start,int rankTarget,uint32_t *pathSet){
    uint8_t queue[BOARD_SIZE];
    uint32_t queued[3] = {0};
    int8_t parent[81];
    Direction directions[] = {LEFT,RIGHT,UP,DOWN};

    queue[0] = start;
    addToSet(start,queued);

    size_t head = 0;
    size_t tail = 1;
    size_t depth = 0;

    while (head < tail){

        int nodesInLevel = tail - head;

        for (size_t i = 0; i < nodesInLevel; i++){
            int8_t pos = queue[head++];
            if (pos >= rankTarget * 9 && pos < (rankTarget + 1) * 9){

                // write path to storage hashset
                if (pathSet != NULL){
                    for (size_t j = 0; j < TILES_HASHSET_LENGTH; j++){
                        pathSet[j] = 0;
                    }

                    int8_t temp = pos;
                    while (temp != start){
                        addToSet(temp,pathSet);
                        temp = parent[temp];
                    }
                    addToSet(temp,pathSet);
                }
                
                return depth;
            }

            int row = pos / 9;
            int column = pos % 9;

            for (size_t j = 0; j < 4; j++){
                Direction dir = directions[j];
                int8_t target = moveDirection(pos,dir);
                if (target != -1 && !inSet(target,queued)){
                    if (!isBlocked(hWalls,vWalls,pos,row,column,dir)){
                        addToSet(target,queued);
                        queue[tail++] = target;
                        parent[target] = pos;
                    }
                }
            }

        }

        depth++;
        
    }

    return -1;
}

bool wallTouchingPath(int32_t *path,int row,int column,bool horizontal){
    int8_t topLeft = row * 9 + column;
    int8_t topRight = MOVE_RIGHT(topLeft);
    if (horizontal){

        if (inSet(topLeft,path) && inSet(MOVE_DOWN(topLeft),path)){
            return true;
        }

        if (inSet(topRight,path) && inSet(MOVE_DOWN(topRight),path)){
            return true;
        }


    } else {

        if (inSet(topLeft,path) && inSet(topRight,path)){
            return true;
        }

        if (inSet(MOVE_DOWN(topLeft),path) && inSet(MOVE_DOWN(topRight),path)){
            return true;
        }

    }

    return false;
}

void calculateRootPath(pBot bot){
    pBoard board = bot->board;
    pPathInfo rootPath = &bot->pathInfos[0];
    rootPath->d1 = bfs(board->hWalls,board->vWalls,board->p1pos,0,rootPath->path1);
    rootPath->d2 = bfs(board->hWalls,board->vWalls,board->p2pos,8,rootPath->path2);
}

bool canPlaceWall(pBot bot,int8_t position,bool horizontal,size_t ply){

    pBoard board = bot->board;
    pPathInfo currentPath = &(bot->pathInfos[ply]);
    pPathInfo nextPath = &(bot->pathInfos[ply + 1]);

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

    int row = position >> 3; // (position / 8)
    int column = position & 0b111; // (position % 8)

    if (horizontal){
        hUpdated |= mask;

        if ((column != 0) && (board->hWalls & (mask >> 1))){
            return false; // wall already placed to the left
        }

        if ((column != 7) && (board->hWalls & (mask << 1))){
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

    bool path1Collision = wallTouchingPath(currentPath->path1,row,column,horizontal);
    bool path2Collision = wallTouchingPath(currentPath->path2,row,column,horizontal);
    
    int depth1 = currentPath->d1;
    if (path1Collision){
        board->bfsCalls++;
        depth1 = bfs(hUpdated,vUpdated,board->p1pos,0,nextPath->path1);
        if (depth1 == -1){
            return false; // can't trap player 1
        }
    } else {
        copyPathSet(currentPath->path1,nextPath->path1);
    }
    
    int depth2 = currentPath->d2;
    if (path2Collision){
        board->bfsCalls++;
        depth2 = bfs(hUpdated,vUpdated,board->p2pos,8,nextPath->path2);
        if (depth2 == -1){
            return false; // can't trap player 2
        }
    } else {
        copyPathSet(currentPath->path2,nextPath->path2);
    }
    

    nextPath->d1 = depth1;
    nextPath->d2 = depth2;

    return true;
}

size_t getLegalMoves(pBot bot,Move *movesBuffer){
    
    pBoard board = bot->board;
    Move *bufferCopy = movesBuffer;

    int8_t movementBuffer[10];
    getPlayerMoves(board,movementBuffer);
    for (int i = 0; movementBuffer[i] != -1; i++){
        Move move = {
            MOVEMENT,
            board->turn == 1 ? board->p1pos : board->p2pos,
            movementBuffer[i]
        };
        *(movesBuffer++) = move;
    }


    for (int8_t i = 0; i < 64; i++){
        if (canPlaceWall(bot,i,true,0)){
            Move move = {
                HORIZONTAL,
                i,
                0
            };
            *(movesBuffer++) = move;
        }
        if (canPlaceWall(bot,i,false,0)){
            Move move = {
                VERTICAL,
                i,
                0
            };
            *(movesBuffer++) = move;
        }
    }

    Move nullMove = {NULL_MOVE,0,0};
    *movesBuffer = nullMove;

    return movesBuffer - bufferCopy;
}

bool isLegalMove(pBot bot, Move *move,int8_t *movementBuffer,size_t ply){
    if (move->moveType == MOVEMENT){
        for (size_t i = 0; movementBuffer[i] != -1; i++){
            if (movementBuffer[i] == move->b2){
                return true;
            }
        }
        return false;
    } else if (move->moveType == HORIZONTAL || move->moveType == VERTICAL){
        return canPlaceWall(bot,move->b1,move->moveType == HORIZONTAL,ply);
    }
    return false;
}
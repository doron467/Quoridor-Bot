#ifndef BOARD_H
#define BOARD_H

// DO NOT CHANGE
#define BOARD_SIZE 81

// can maybe change
#define P1_START_POS 76
#define P2_START_POS 4
#define INITIAL_WALL_COUNT 10

#define LEFT(pos) ((pos) - 1)
#define RIGHT(pos) ((pos) + 1)
#define UP(pos) ((pos) - 9)
#define DOWN(pos) ((pos) + 9)

#define CAN_LEFT(pos) ((pos) % 9 != 0)
#define CAN_RIGHT(pos) ((pos) % 9 != 8)
#define CAN_UP(pos) (((pos) - 9) >= 0)
#define CAN_DOWN(pos) (((pos) + 9) < 81)

#define SWITCH_TURNS(boardPointer) ((boardPointer)->turn = (boardPointer)->turn % 2 + 1)

#include <stdint.h>
#include <stdbool.h>

typedef struct _Board {
    uint64_t hWalls;
    uint64_t vWalls;
    uint8_t p1wc; // p1 wall count
    uint8_t p2wc; // p2 wall count
    int8_t p1pos; // 0 indexed from top left
    int8_t p2pos; // 0 indexed from top left
    uint8_t turn; // 1 if p1 turn, 2 if p2 turn
} Board;

typedef Board *pBoard;

pBoard initializeBoard();
void printBoard(pBoard);
int bfs(uint64_t hWalls,uint64_t vWalls,int8_t start,int rankTarget); // returns -1 if no path found, otherwise returns length of the path
bool placeWall(pBoard board,int8_t position,bool horizontal); // returns if the placement succeeded or not

#endif
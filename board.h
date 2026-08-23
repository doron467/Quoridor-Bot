#ifndef BOARD_H
#define BOARD_H

// DO NOT CHANGE
#define BOARD_SIZE 81

// can maybe change
#define P1_START_POS 76
#define P2_START_POS 4
#define INITIAL_WALL_COUNT 10

#define MOVE_LEFT(pos) ((pos) - 1)
#define MOVE_RIGHT(pos) ((pos) + 1)
#define MOVE_UP(pos) ((pos) - 9)
#define MOVE_DOWN(pos) ((pos) + 9)

#define CAN_LEFT(pos) ((pos) % 9 != 0)
#define CAN_RIGHT(pos) ((pos) % 9 != 8)
#define CAN_UP(pos) (((pos) - 9) >= 0)
#define CAN_DOWN(pos) (((pos) + 9) < 81)

#define SWITCH_TURNS(boardPointer) ((boardPointer)->turn = (boardPointer)->turn % 2 + 1)

#include <stdint.h>
#include <stdbool.h>

typedef enum _Direction {LEFT,RIGHT,UP,DOWN} Direction;
typedef enum _MoveType {MOVEMENT,HORIZONTAL,VERTICAL} MoveType;

typedef struct _Board {
    uint64_t hWalls; // hashmap for horizontal walls
    uint64_t vWalls; // hashmap for vertical walls
    uint8_t p1wc; // p1 wall count
    uint8_t p2wc; // p2 wall count
    int8_t p1pos; // 0 indexed from top left
    int8_t p2pos; // 0 indexed from top left
    uint8_t turn; // 1 if p1 turn, 2 if p2 turn
} Board;

typedef Board *pBoard;


/*
when move type is MOVEMENT, b1 is where the player was, and b2 is where the player went.
when move type is HORIZONTAL/VERTICAL, b1 is the position of the wall. b2 is garbage
*/
typedef struct _Move {
    MoveType moveType;
    int8_t b1;
    int8_t b2;
} Move;

pBoard initializeBoard();

void printBoard(pBoard);

/*
makes a move on the board.
no safety checks are done, to keep the function fast.
it's the responsibility of the caller to check if a move is legal or not before calling the function.
*/
void makeMove(pBoard board,Move *move);

/*
unmakes a move on the board.
no safety checks are done, to keep the function fast.
it's the responsibility of the caller to check if a move is legal or not before calling the function.
*/
void unmakeMove(pBoard board, Move *move);

// returns -1 if no path found, otherwise returns length of the path
int bfs(uint64_t hWalls,uint64_t vWalls,int8_t start,int rankTarget);

// returns if the placement succeeded or not
bool canPlaceWall(pBoard board,int8_t position,bool horizontal);

/*
writes all possible player moves into the buffer, with -1 as the last move.
the minimum safe length for the buffer is 6. recommendation is 10
*/
void getPlayerMoves(pBoard board,int8_t *buffer);

#endif
#ifndef BOARD_H
#define BOARD_H

// DO NOT CHANGE
#define BOARD_SIZE 81
#define TILES_HASHSET_LENGTH 3

// can maybe change
#define MAX_GAME_LENGTH 500
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

typedef struct _Bot Bot;
typedef Bot *pBot;

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef enum _Direction {LEFT,RIGHT,UP,DOWN} Direction;
typedef enum _MoveType {MOVEMENT,HORIZONTAL,VERTICAL,NULL_MOVE} MoveType;

/*
when move type is MOVEMENT, b1 is where the player was, and b2 is where the player went.
when move type is HORIZONTAL/VERTICAL, b1 is the position of the wall. b2 is garbage
*/
typedef struct _Move {
    MoveType moveType;
    int8_t b1;
    int8_t b2;
} Move;


// effectively an 81 bitmap for the board tiles
typedef struct _TileSet {
    uint32_t bits[TILES_HASHSET_LENGTH];
} TileSet;

typedef TileSet *pTileSet;

typedef struct _Board {
    uint64_t hWalls; // hashset for horizontal walls
    uint64_t vWalls; // hashset for vertical walls
    uint8_t p1wc; // p1 wall count
    uint8_t p2wc; // p2 wall count
    int8_t p1pos; // 0 indexed from top left
    int8_t p2pos; // 0 indexed from top left
    uint8_t turn; // 1 if p1 turn, 2 if p2 turn
    Move moveHistory[MAX_GAME_LENGTH];
    size_t lastMoveIndex;

    uint64_t bfsCalls; // debugging
} Board;

typedef Board *pBoard;


pBoard initializeBoard(uint8_t turn);
void printBoard(pBoard board);
bool isGameOver(pBoard board);
void printMove(Move *move);
void printMoveHistory(pBoard board);
bool sameMove(Move *move1, Move *move2);

bool inSet(int8_t pos, const uint32_t *hashset);
void addToSet(int8_t pos, uint32_t *hashset);
void removeFromSet(int8_t pos,uint32_t *hashset);
void removeNeighboursFromSet(pBoard board,int8_t pos,uint32_t *hashset);

/*
makes a move on the board.
no safety checks are done, to keep the function fast.
it's the responsibility of the caller to check if a move is legal or not before calling the function.
*/
void makeMove(pBot bot,Move *move,size_t ply);

/*
unmakes a move on the board.   
no safety checks are done, to keep the function fast.
it's the responsibility of the caller to check if a move is legal or not before calling the function.
*/
void unmakeMove(pBoard board, Move *move);

// returns -1 if no path found, otherwise returns length of the path
// path storage is a hashset where the resulting path will be written
int bfs(uint64_t hWalls,uint64_t vWalls,int8_t start,int rankTarget,uint32_t *pathStorage);

void calculateRootPath(pBot bot);

bool wallCollidingPath(int32_t *path,int row,int column,bool horizontal);

bool placementAvailable(pBoard board,int row,int column,bool horizontal);

// returns if the placement succeeded or not
bool canPlaceWall(pBot bot,int8_t position,bool horizontal,size_t ply);

/*
writes all possible player moves into the buffer, with -1 as the last move.
the minimum safe length for the buffer is 6. recommendation is 10.
also returns the amount of legal squares the player can move to
*/
size_t getPlayerMoves(pBoard board,int8_t *buffer);

/*
same as getPlayerMoves, but returns ALL moves, with wall placements included.
min length for the buffer is 134 (maybe). recommended length is 200
note that the buffer here is a struct move, while in getPlayerMoves it's an int
*/
size_t getLegalMoves(pBot bot,Move *movesBuffer);


/*
returns if a move is legal or not. movement buffer is the buffer from getPlayerMoves
*/
bool isLegalMove(pBot bot, Move *move,int8_t *movementBuffer,size_t ply);

#endif
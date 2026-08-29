#define THINKING_TIME_LIMIT 100 // how long (in seconds) is the agent allowed to think per turn
#define TT_SIZE (1 << 20)   // transpositions table size (in entries) (~ 1 million)
#define CLOCK_CHECK 1024 // how many nodes before doing a clock check

// do not change
#define HORIZONTAL_ZOBRIST_INDEX 0
#define VERTICAL_ZOBRIST_INDEX 1

#include "board.h"
#include <stdlib.h>
#include <time.h>

typedef struct _Deadline {
    clock_t deadline; // clock deadline for the search
    uint64_t nodesCount; // helper for limiting the clock check to every multiple checks
    bool timeExpired;
} Deadline;

typedef struct _ScoredMove {
    Move move;
    int score;
} ScoredMove;

typedef struct _ZobristValues {
    uint64_t wallHash[2][64]; // 0 index for horizontals, 1 for verticals
    uint64_t playerHash[2][81];
    uint64_t wallCountHash[2][11];
    uint64_t turnHash[2];
} ZobristValues;

typedef enum _TTFlag {
    TT_EXACT,
    TT_UPPER_BOUND,
    TT_LOWER_BOUND
} TTFlag;

typedef struct _TTEntry {
    uint64_t key;
    int score;
    int depth;
    Move bestMove;
    TTFlag flag;
} TTEntry;

typedef struct _Bot {
    pBoard board;
    ZobristValues *zobristValues;
    uint64_t boardHash;
    TTEntry transpositionsTable[TT_SIZE];

    uint64_t ttHits;
    uint64_t ttMatches;
    uint64_t ttStores;
    uint64_t ttCutoffs;
} Bot;

typedef Bot *pBot;

/*
creates a bot with a given board pointer.
the bot initializes data it needs like zobrist values for hashing
*/
pBot createBot(pBoard board);

// frees the bot from memory
void destroyBot(pBot bot);

/*
updates the bot's board hash incrementally with a given move.
works both for making and unmaking a move.
this function should be called BEFORE MAKING a move, and AFTER UNMAKING a move.
*/
void updateHash(pBot bot, Move *move);

// calculates the hash for the board from scratch
void calculateBoardHash(pBot bot);

// writes the bot's best move into the move buffer
void getBestMove(pBot bot,Move *buffer);
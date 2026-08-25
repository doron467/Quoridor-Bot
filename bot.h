#define THINKING_TIME_LIMIT 5 // how long (in seconds) is the agent allowed to think per turn
#define CLOCK_CHECK 1024 // how many nodes before doing a clock check

#include "board.h"
#include <stdlib.h>

typedef struct _Deadline {
    clock_t deadline; // clock deadline for the search
    uint64_t nodesCount; // helper for limiting the clock check to every multiple checks
    bool timeExpired;
} Deadline;

typedef struct _ScoredMove {
    Move move;
    int score;
} ScoredMove;

void getBestMove(pBoard board,Move *buffer);
#include "../board.h"
#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>

bool readBoardFEN(Board *board);
bool readMove(Move *move);
void printBoardFEN(const Board *board);
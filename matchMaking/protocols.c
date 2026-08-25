#include "protocols.h"

void printBoardFEN(const Board *board){
    printf(
                "OK %d %d %" PRIu64 " %" PRIu64
                " %d %d %d\n",

                board->p1pos,
                board->p2pos,
                board->hWalls,
                board->vWalls,
                board->p1wc,
                board->p2wc,
                board->turn
            );
}

bool readBoardFEN(Board *board) {
    return scanf(
        "%hhd %hhd %" SCNu64 " %" SCNu64 " %hhd %hhd %hhd",
        &board->p1pos,
        &board->p2pos,
        &board->hWalls,
        &board->vWalls,
        &board->p1wc,
        &board->p2wc,
        &board->turn
    ) == 7;
}

bool readMove(Move *move){

    int moveType;

    int args = scanf(
            "%d %hhd %hhd",
            &moveType,
            //&move->moveType,
            &move->b1,
            &move->b2
        );
    
    if (args != 3){return false;}

    move->moveType = moveType;
    return true;
}
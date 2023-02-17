
#include "./GameState.h"

#include <iostream>



int main()
{
    GameState game;
    GameState game2{ game };
    game.playGame();
    return 0;
}
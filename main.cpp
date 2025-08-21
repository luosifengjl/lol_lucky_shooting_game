#define SDL_MAIN_HANDLED

#include "gameManager.hpp"

int main(int argc,char** argv){
    GameManager& gameManager = GameManager::getInstance();
    gameManager.gameInit();
    gameManager.mainloop();
    return 0;
}


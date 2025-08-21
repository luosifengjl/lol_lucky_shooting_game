#pragma once

#include "monster.hpp"
#include "gameManager.hpp"

class MonsterFast : public Monster{

public:
    MonsterFast() {
        speed_run = 80.f;
    }

    ~MonsterFast() = default;
};

class MonsterSlow : public Monster{

    public:
        MonsterSlow() {
            speed_run = 30.f;
        }

        ~MonsterSlow() = default;
};


class MonsterMedium : public Monster{

    public:
        MonsterMedium() {
           
            speed_run = 50.f;
        }

        ~MonsterMedium() = default;
};

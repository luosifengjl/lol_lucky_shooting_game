#pragma once

#include "monster.hpp"

extern Atlas atlas_monster_fast;
extern Atlas atlas_monster_slow;
extern Atlas atlas_monster_medium;

class MonsterFast : public Monster{

public:
    MonsterFast() {
        animation_run.add_frame(&atlas_monster_fast);
        speed_run = 80.f;
    }

    ~MonsterFast() = default;
};

class MonsterSlow : public Monster{

    public:
        MonsterSlow() {
            animation_run.add_frame(&atlas_monster_slow);
            speed_run = 30.f;
        }

        ~MonsterSlow() = default;
};


class MonsterMedium : public Monster{

    public:
        MonsterMedium() {
            animation_run.add_frame(&atlas_monster_medium);
            speed_run = 50.f;
        }

        ~MonsterMedium() = default;
};

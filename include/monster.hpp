#pragma once

#include "camera.hpp"
#include "vector2.hpp"
#include "animation.hpp"

#include <SDL3_mixer/SDL_mixer.h>

extern Atlas atlas_explosion;

extern MIX_Audio* sound_explosion;

class Monster{
    public:
        Monster(){
            animation_run.set_loop(true);
            animation_run.set_interval(0.1f);
            animation_explosion.set_loop(false);
            animation_explosion.set_interval(0.08f);
            animation_explosion.add_frame(&atlas_explosion);
            animation_explosion.set_on_finished([&](){
                is_valid = false; // 爆炸动画结束后，标记为无效
            });
            position.x = 40.0f + (rand() % 1200);
            position.y = -50;
        }
        ~Monster() = default;

        const Vector2& get_position() const {
            return position;
        }

        void on_update(float delta){
            if (!is_alive) {
                position.y +=speed_run * delta;
            }
            animation_current = (is_alive) ? &animation_run : &animation_explosion;
            animation_current->set_position(position);
            animation_current->on_update(delta);
        }

        void on_render(const Camera& camera) const {
            animation_current->on_render(camera);
        }

        void on_hurt()
        {
            is_alive = false;
            MIX_PlayAudio(NULL,sound_explosion);
        }

        void make_invalid()
        {
            is_valid = false; // 标记为无效
        }

        bool check_alive() const
        {
            return is_alive;
        }

        bool can_remove() const{
            return !is_valid;
        }
    protected:
        float speed_run = 10.0f;
        Animation animation_run;
    
    private:
        Vector2 position;
        Animation animation_explosion;
        Animation* animation_current = nullptr;

        bool is_alive = true; // 是否存活
        bool is_valid = true; // 是否有效
};
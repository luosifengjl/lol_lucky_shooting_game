#pragma once

#include "camera.hpp"
#include "vector2.hpp"

#include <SDL3/SDL.h>

#define M_PI 3.14159265358979323846

extern SDL_Texture* tex_bullet;

class Bullet{
    public:
        Bullet(double angle)
        {
            this->angle = angle;
            double radian = angle * M_PI / 180.0;
            velocity.x = std::cos(radian) * speed;
            velocity.y = std::sin(radian) * speed;
        }
        ~Bullet() = default;

        void set_position(const Vector2& pos){
            position = pos;
        }

        Vector2 get_position(){
            return position;
        }
        
        void on_update(float delta_time){
            position += velocity * delta_time;
            if(position.x <= 0 || position.x >= 1280 || position.y <= 0 || position.y >=720) {
                is_valid = false; // 超出屏幕范围，标记为不合法
            }
        }

        void on_render(const Camera& camera) const{
            const SDL_FRect rect_bullet = {position.x - 4,position.y - 5,8,4};
            camera.render_texture(tex_bullet, nullptr, &rect_bullet, angle, nullptr);
        }

        void on_hit()
        {
            is_valid = false;
        }

        bool can_remove() const
        {
            return !is_valid;
        }

private:
        double angle = 0.0;
        Vector2 position;
        Vector2 velocity;
        bool is_valid = true; // 是否有效
        float speed = 800.0f; // 子弹速度
};
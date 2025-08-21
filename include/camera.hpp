#pragma once


#include "timer.hpp"
#include "vector2.hpp"

#include <SDL3/SDL.h>


//SDL3中renderer 是一个指向SDL渲染器对象的指针 理解为画布+画笔 主要用于绘制纹理 rendererTexture
class Camera {
    public:
    Camera(SDL_Renderer* renderer){
        this->renderer = renderer;
        timer_shake.set_one_shot(true);
        timer_shake.set_on_timeout([&](){
            is_shaking = false;
            reset();
        });
    }
    ~Camera() = default;

    const Vector2& get_position() const {
        return position;
    }

    void reset(){
        position.x = 0.0f;
        position.y = 0.0f;
    }

    void on_update(float delta_time){
        timer_shake.on_update(delta_time);
        if (is_shaking) {
            position.x = (-50 + rand() % 100) / 50.0f * shaking_strength;
            position.y =  (-50 + rand() % 100) / 50.0f * shaking_strength;
        }
    }

    void shake(float strength, float duration){
        is_shaking = true;
        shaking_strength = strength;
        timer_shake.set_wait_time(duration);
        timer_shake.restart();
    }
    
    //rectangle 矩形
    void render_texture(SDL_Texture* texture,  const SDL_FRect* rect_src,
        const SDL_FRect* rect_dst,double angle,const SDL_FPoint* center)  const {
        SDL_FRect rect_dst_win = *rect_dst;
        rect_dst_win.x -= position.x;
        rect_dst_win.y -= position.y;
        SDL_RenderTextureRotated(renderer, texture, rect_src,&rect_dst_win,angle,center, SDL_FLIP_NONE);
    }

    private:
    Vector2 position;
    Timer timer_shake;
    bool is_shaking = false;
    float shaking_strength = 0.0f;
    SDL_Renderer* renderer = nullptr;
};
#pragma once

#include "timer.hpp"
#include "atlas.hpp"
#include "camera.hpp"
#include "vector2.hpp"

#include <SDL3/SDL.h>

#include <vector>
#include <functional>

class Animation{
    public:
        Animation(){
            timer.set_one_shot(false);
            timer.set_on_timeout([&](){
                idx_frame++;
                if (idx_frame >= frame_list.size()) {
                    idx_frame = is_loop ? 0 : frame_list.size() - 1;
                    if(!is_loop && on_finished){
                        on_finished();
                    }
                }
            });

        }
        ~Animation() = default;

        void reset(){
            timer.restart();
            idx_frame = 0;
        }

        void set_position(const Vector2& pos){
            position = pos;
        }

        void set_rotation(double angle){
            angle = angle;
        }

        void set_center(const SDL_FPoint& center){
           this-> center = center;
        }

        void set_loop(bool loop){
            is_loop = loop;
        }

        void set_interval(float interval){
            timer.set_wait_time(interval);
        }

        void set_on_finished(std::function<void()> func){
            on_finished = func;
        }

        void add_frame(SDL_Texture* texture,int num_h){
            float width,height;
            SDL_GetTextureSize(texture, &width, &height);

            float width_frame =width / num_h;
            for(int i = 0 ; i < num_h;i++){
                SDL_FRect rect_src;
                rect_src.x = i * width_frame;
                rect_src.y = 0;
                rect_src.w = width_frame;
                rect_src.h = height;
                frame_list.emplace_back(texture, rect_src);
            }
        }

        void add_frame(Atlas* atlas){
            for(int i = 0 ; i < atlas->get_size();i++){
                    SDL_Texture* texture = atlas->get_texture(i);
                    float width,height;
                    SDL_GetTextureSize(texture, &width, &height);
                    SDL_FRect rect_src;
                    rect_src.x = 0;
                    rect_src.y = 0;
                    rect_src.w = width;
                    rect_src.h = height;
                    frame_list.emplace_back(texture, rect_src);
             }
         }

         void on_update(float delta_time){
            timer.on_update(delta_time);
        }

        void on_render(const Camera& camera) const{
            const Frame& frame = frame_list[idx_frame];
            const Vector2& pos_camera = camera.get_position();
            SDL_FRect rect_dst;
            rect_dst.x = position.x - frame.rect_src.w / 2;
            rect_dst.y = position.y - frame.rect_src.h / 2;
            rect_dst.w = frame.rect_src.w;
            rect_dst.h = frame.rect_src.h;
            camera.render_texture(frame.texture, &frame.rect_src, &rect_dst, angle, &center);
        }
    private:
        struct Frame {
            SDL_Texture* texture;
            SDL_FRect rect_src;
            Frame() = default;
            Frame(SDL_Texture* tex, const SDL_FRect& rect) : texture(tex), rect_src(rect) {}
            ~Frame() = default;
        };

    private:
        std::vector<Frame> frame_list;
        Vector2 position;
        double angle = 0.0;
        SDL_FPoint center = {0.0f, 0.0f};
        Timer timer;
        int idx_frame = 0;
        bool is_loop = true;
        std::function<void()> on_finished = nullptr;
};
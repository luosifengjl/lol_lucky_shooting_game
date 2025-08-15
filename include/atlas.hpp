#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <string>
#include <vector>

class Atlas{
    public:
        Atlas() = default;
        ~Atlas() {
            for(SDL_Texture* texture : tex_list) {
                SDL_DestroyTexture(texture);
            }
        }
        void load(SDL_Renderer* renderer, const char* path_template,int num) {

            for(int i = 0 ; i< num ;i++){
                char path_file[256];
                sprintf(path_file, path_template, i+ 1); //sprintf 格式化字符串 stirng+ printf (打印到字符串) 第二个参数是格式化字符串 类似于img_%d
                SDL_Texture* texture = IMG_LoadTexture(renderer, path_file);
                tex_list.push_back(texture);
            }
        }

        void clear(){
            tex_list.clear();
        }

        int get_size() const {
            return tex_list.size();
        }

        SDL_Texture* get_texture(int index) const {
            if (index < 0 || index >= tex_list.size()) {
                return nullptr; 
            }
            return tex_list[index];
        }

        void add_texture(SDL_Texture* texture) {
                tex_list.push_back(texture);
        }
    private:
        std::vector<SDL_Texture*> tex_list; //存储纹理的列表
};
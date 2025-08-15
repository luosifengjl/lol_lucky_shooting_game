#define SDL_MAIN_HANDLED

#include "include/atlas.hpp"
#include "include/camera.hpp"
#include "include/bullet.hpp"
#include "include/monster.hpp"
#include "include/monster_child.hpp"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <algorithm>

Camera* camera = nullptr;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

bool is_quit = false;

SDL_Texture* tex_heart = nullptr;
SDL_Texture* tex_bullet = nullptr;
SDL_Texture* tex_battery = nullptr;
SDL_Texture* tex_crosshair = nullptr;
SDL_Texture* tex_background = nullptr;
SDL_Texture* tex_barrel_idle = nullptr;

Atlas atlas_barrel_fire;
Atlas atlas_monster_fast;
Atlas atlas_monster_medium;
Atlas atlas_monster_slow;
Atlas atlas_explosion;

MIX_Audio* music_bgm = nullptr;
MIX_Audio*  music_loss = nullptr;

MIX_Audio*  sound_hurt = nullptr;
MIX_Audio*  sound_fire_1 = nullptr;
MIX_Audio*  sound_fire_2 = nullptr;
MIX_Audio*  sound_fire_3 = nullptr;
MIX_Audio*  sound_explosion = nullptr;

TTF_Font* font = nullptr;

int hp = 10;
int score = 0;
std::vector<Bullet> bullet_list;
std::vector<Monster*> monster_list;

int num_per_gen = 2;
Timer timer_generate;
Timer timer_increase_num_per_gen;

Vector2 pos_crosshair;
double angle_barrel = 0;
const Vector2 pos_battery = {640,600};
const Vector2 pos_barrel = {592,585};
const SDL_FPoint center_barrel = {48,25};

bool is_cool_down = true;
bool is_fire_key_down = false;
Animation animation_barrel_fire;

void load_resources();
void unload_resources();
void init();
void deinit();
void on_update(float delta);
void on_render(const Camera& camera);
void mainloop();

int main(int argc,char** argv){
    init();
    mainloop();
    deinit();
    return 0;
}

void init(){
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    MIX_Init();
    TTF_Init();
    MIX_Mixer* mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    MIX_Track* tracks[32];
    for (int i = 0; i < 32; ++i) {
        tracks[i] = MIX_CreateTrack(mixer);
    }
    window = SDL_CreateWindow(u8"《demo》",1280,720 ,SDL_WINDOW_RESIZABLE );
    
    renderer = SDL_CreateRenderer(window,NULL);

    SDL_HideCursor();

    load_resources();

    camera = new Camera(renderer);

    timer_generate.set_one_shot(false);
    timer_generate.set_wait_time(1.5f);
    timer_generate.set_on_timeout([&]()
        {
            for(int i = 0 ; i < num_per_gen ; i++){
                int val = rand() % 100;
                Monster* monster = nullptr;
                if(val < 50)
                    monster = new MonsterSlow();
                else if(val < 80)
                    monster = new MonsterMedium();
                else 
                    monster = new MonsterFast();
                monster_list.push_back(monster);
            }
        });
    timer_increase_num_per_gen.set_one_shot(false);
    timer_increase_num_per_gen.set_wait_time(8.0f);
    timer_increase_num_per_gen.set_on_timeout([&]()
        {
            num_per_gen += 1;
        });

    animation_barrel_fire.set_loop(false);  //炮管开火
    animation_barrel_fire.set_interval(0.04f);
    animation_barrel_fire.set_center(center_barrel);
    animation_barrel_fire.add_frame(&atlas_barrel_fire);
    animation_barrel_fire.set_on_finished([&](){
        is_cool_down = true;
    });
    animation_barrel_fire.set_position({718,610});

    MIX_PlayAudio(mixer,music_bgm);
    
}

void deinit(){
    delete camera;
    unload_resources();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_Quit();
    MIX_Quit();
    SDL_Quit();
}

void load_resources(){
        tex_heart = IMG_LoadTexture(renderer,"resources/heart.png");
        tex_bullet = IMG_LoadTexture(renderer,"resources/bullet.png");
        tex_battery = IMG_LoadTexture(renderer,"resources/battery.png");
        tex_crosshair = IMG_LoadTexture(renderer,"resources/crosshair.png");
        tex_background = IMG_LoadTexture(renderer,"resources/background.png");
        tex_barrel_idle = IMG_LoadTexture(renderer,"resources/barrel_idle.png");

        atlas_barrel_fire.load(renderer,"resources/barrel_fire_%d.png",3);
        atlas_monster_fast.load(renderer,"resources/monster_fast_%d.png",4);
        atlas_monster_medium.load(renderer,"resources/monster_medium_%d.png",6);
        atlas_monster_slow.load(renderer,"resources/monster_slow_%d.png",8);
        atlas_explosion.load(renderer,"resources/explosion_%d.png",5);

        music_bgm = MIX_LoadAudio(NULL,"resources/bgm.mp3",false);
        music_loss =  MIX_LoadAudio(NULL,"resources/loss.mp3",false);

        sound_hurt = MIX_LoadAudio(NULL,"resources/hurt.wav",true);
        sound_fire_1 = MIX_LoadAudio(NULL,"resources/fire_1.wav",true);
        sound_fire_2 = MIX_LoadAudio(NULL,"resources/fire_2.wav",true);
        sound_fire_3 = MIX_LoadAudio(NULL,"resources/fire_3.wav",true);
        sound_explosion = MIX_LoadAudio(NULL,"resources/explosion.wav",true);

        font = TTF_OpenFont("resources/IPix.ttf",28);
}

void unload_resources(){
    SDL_DestroyTexture(tex_heart);
    SDL_DestroyTexture(tex_bullet);
    SDL_DestroyTexture(tex_battery);
    SDL_DestroyTexture(tex_crosshair);
    SDL_DestroyTexture(tex_background);
    SDL_DestroyTexture(tex_barrel_idle);
    
    MIX_DestroyAudio(music_bgm);
    MIX_DestroyAudio(music_loss);

   MIX_DestroyAudio(sound_hurt);
   MIX_DestroyAudio(sound_fire_1);
   MIX_DestroyAudio(sound_fire_2);
   MIX_DestroyAudio(sound_fire_3);
   MIX_DestroyAudio(sound_explosion);
}

void mainloop(){
    using namespace std::chrono;

    SDL_Event event;

    const nanoseconds frame_duration(1000000000/144);
    steady_clock::time_point last_tick = steady_clock::now();

    while(!is_quit){
        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_EVENT_QUIT:
                    is_quit = true;
                    break;
                case SDL_EVENT_MOUSE_MOTION:
                {
                    pos_crosshair.x = static_cast<float>(event.motion.x);
                    pos_crosshair.x = static_cast<float>(event.motion.y);
                    Vector2 direction = pos_crosshair - pos_battery;
                    angle_barrel = std::atan2(direction.y,direction.x) * 180 / 3.14159265;
                }
                    break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    is_fire_key_down = true;
                case SDL_EVENT_MOUSE_BUTTON_UP:
                    is_fire_key_down = false;
                    break;
            }
        }
        steady_clock::time_point frame_start = steady_clock::now();
        duration<float> delta = duration<float>(frame_start - last_tick);

        on_update(delta.count());

        on_render(*camera);

        SDL_RenderPresent(renderer);

        last_tick = frame_start;
        nanoseconds sleep_duration = frame_duration - (steady_clock::now() - frame_start);
        if(sleep_duration > nanoseconds(0))
            std::this_thread::sleep_for(sleep_duration);
    }
}

void on_update(float delta){
    timer_generate.on_update(delta);
    timer_increase_num_per_gen.on_update(delta);

    //更新子弹列表
    for(Bullet& bullet : bullet_list){
        bullet.on_update(delta);
    }

    //更新怪物对象 处理子弹逻辑
    for(Monster* monster : monster_list){
        monster->on_update(delta);
        for(Bullet& bullet : bullet_list){
            if(!monster->check_alive() || bullet.can_remove()) continue;
            const Vector2& pos_bullet = bullet.get_position();
            const Vector2& pos_monster = monster->get_position();
            static const Vector2 size_monster ={30,40};
            if(pos_bullet.x >= pos_monster.x - size_monster.x / 2
            && pos_bullet.x <= pos_monster.x + size_monster.x /2
            && pos_bullet.y >= pos_monster.y + size_monster.y / 2)
            {
                score += 1;
                bullet.on_hit();
                monster->on_hurt();
            }
        }
        if(!monster->check_alive()) continue;

        //过线会减少生命值
        if(monster->get_position().y >= 720){
            monster->make_invalid();
            MIX_PlayAudio(NULL,sound_hurt);
            hp -= 1;
        }
    }

    //移除无效子弹对象
    bullet_list.erase(std::remove_if(
        bullet_list.begin(),bullet_list.end(),
        [](const Bullet& bullet){
            return bullet.can_remove();
        }),
        bullet_list.end());

        //移除无效怪物对象
        monster_list.erase(std::remove_if(
            monster_list.begin(),monster_list.end(),
            [](Monster* monster)
            {
                bool can_remove = monster->can_remove();
                if(can_remove) delete monster;
                return can_remove;
            }
        ),monster_list.end());
    
    std::sort(monster_list.begin(),monster_list.end(),
        [](const Monster* monster_1,const Monster* monster_2)
        {return monster_1->get_position().y < monster_2->get_position().y;});
    
    //处理开火逻辑
    if(!is_cool_down){
        camera->shake(3.0f,0.1f);
        animation_barrel_fire.on_update(delta);
    }
    //处理开火瞬间逻辑
    if(is_cool_down && is_fire_key_down){
        animation_barrel_fire.reset();
        is_cool_down = false;

        static const float length_barrel = 105;
        static const Vector2 pos_barrel_center = {640,610};

        bullet_list.emplace_back(angle_barrel);
        Bullet& bullet = bullet_list.back();
        double angle_bullet = angle_barrel + (rand() % 30 - 15);
        double radians = angle_bullet * 3.14159265 / 180;
        Vector2 direction = {static_cast<float>(std::cos(radians)),static_cast<float>(std::sin(radians))};
        bullet.set_position(pos_barrel_center + direction * length_barrel);
        switch(rand() % 3){
            case 0 : MIX_PlayAudio(NULL,sound_fire_1); break;
            case 1 : MIX_PlayAudio(NULL,sound_fire_2); break;
            case 2 : MIX_PlayAudio(NULL,sound_fire_3); break;
        }
    }

    //更新摄像机位置
    camera->on_update(delta);

    //检查游戏是否结束
    if(hp <= 0){
        is_quit = true;
        MIX_StopTrack(NULL,0);
        MIX_PlayAudio(NULL,music_loss);
        std::string msg = u8"游戏最终得分：" + std::to_string(score);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, u8"游戏结束",msg.c_str(),window);

    }
}

void on_render(const Camera& camera){
    {
        float width_bg,height_bg;
        SDL_GetTextureSize(tex_background,&width_bg,&height_bg);
        const SDL_FRect rect_background = 
        {
            (1280 - width_bg) / 2.0f,
            (720 - height_bg) / 2.0f,
            static_cast<float>(width_bg),static_cast<float>(height_bg)
        };
        camera.render_texture(tex_background,nullptr,&rect_background,0,nullptr);
    }

    //绘制怪物
    for(Monster* monster : monster_list)  monster->on_render(camera);

    //绘制子弹
    for(const Bullet& bullet : bullet_list){
        bullet.on_render(camera);
    }

    //绘制炮台
    {
        //绘制炮台底座
        float width_battery,height_battery;
        SDL_GetTextureSize(tex_battery,&width_battery,&height_battery);
        const SDL_FRect rect_battery = 
        {
            pos_battery.x - width_battery / 2.0f,
            pos_battery.y - height_battery / 2.0f,
            static_cast<float>(width_battery),static_cast<float>(height_battery)
        };
        camera.render_texture(tex_battery,nullptr,&rect_battery,0,nullptr);

        //绘制炮管
        float width_barrel,height_barrel;
        SDL_GetTextureSize(tex_barrel_idle,&width_barrel,&height_barrel);
        const SDL_FRect rect_barrel = 
        {
            pos_barrel.x - width_barrel / 2.0f,
            pos_barrel.y - height_barrel / 2.0f,
            static_cast<float>(width_barrel),static_cast<float>(height_barrel)
        };
        if(is_cool_down)
            camera.render_texture(tex_barrel_idle,nullptr,&rect_barrel,angle_barrel,&center_barrel);
        else{
            animation_barrel_fire.set_rotation(angle_barrel);
            animation_barrel_fire.on_render(camera);
        }
    }

    //绘制生命值
    {
        float width_heart,height_heart;
        SDL_GetTextureSize(tex_heart,&width_heart,&height_heart);
        for(int i = 0 ; i < hp;i++){
            const SDL_FRect rect_dst = 
            {
                15 + (width_heart + 10) * i , 15,
                width_heart,height_heart
            };
            SDL_RenderTexture(renderer, tex_heart, NULL, &rect_dst);
        }
    }

    //绘制游戏得分
    {
        std::string str_score = "SCORE: "  + std::to_string(score);
        SDL_Surface* suf_score_bg = TTF_RenderText_Blended(font,str_score.c_str(),0,SDL_Color({55,55,55,255}));
        SDL_Surface* suf_score_fg = TTF_RenderText_Blended(font,str_score.c_str(),0,{255,255,255,255});
        SDL_Texture* tex_score_bg = SDL_CreateTextureFromSurface(renderer,suf_score_bg);
        SDL_Texture* tex_score_fg = SDL_CreateTextureFromSurface(renderer,suf_score_fg);
        SDL_FRect rect_dst_score = {1280 - suf_score_bg->w - 15,15,suf_score_bg->w,suf_score_bg->h};
        SDL_RenderTexture(renderer,tex_score_bg,nullptr,&rect_dst_score);
        rect_dst_score.x -=2,rect_dst_score.y -= 2;
        SDL_RenderTexture(renderer,tex_score_fg,nullptr,&rect_dst_score);
        SDL_DestroyTexture(tex_score_bg); SDL_DestroyTexture (tex_score_fg);
        SDL_DestroySurface(suf_score_bg); SDL_DestroySurface(suf_score_fg);
         
    }

    //绘制准星
    {
        float width_crosshair,height_crosshair;
        SDL_GetTextureSize(tex_crosshair,&width_crosshair,&height_crosshair);
        const SDL_FRect rect_crosshair = 
        {
            pos_crosshair.x - width_crosshair / 2.0f,
            pos_crosshair.y - height_crosshair / 2.0f,
            static_cast<float>(width_crosshair),static_cast<float>(height_crosshair)
        };
        camera.render_texture(tex_crosshair,nullptr,&rect_crosshair,0,nullptr);
    }
}  
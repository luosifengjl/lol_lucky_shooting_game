#pragma once

#include "vector2.hpp"
#include "bullet.hpp"
#include "monster.hpp"
#include "monster_child.hpp"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <chrono>
#include <vector>
#include <algorithm>
#include <thread>

class AudioManager{
    public:
    static AudioManager& getInstance() {
        static AudioManager instance;  //static 修饰的成员变量 只有第一个会初始化 剩下时间不会初始化
        if(!instance.initialized){
            instance.init();
            instance.initialized = true;
        }
        return instance;
    }
    
    //获取
    MIX_Mixer* getMixer() const {
        return mixer;
    }
    //设置
    void setMixer( MIX_Mixer* new_mixer) {
        mixer = new_mixer;
    }
    
    void init() {
        if(initialized) return;
        mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,NULL);
     
    }
    
    MIX_Audio* music_bgm = nullptr;
    MIX_Audio*  music_loss = nullptr;

    MIX_Audio*  sound_hurt = nullptr;
    MIX_Audio*  sound_fire_1 = nullptr;
    MIX_Audio*  sound_fire_2 = nullptr;
    MIX_Audio*  sound_fire_3 = nullptr;
    MIX_Audio*  sound_explosion = nullptr;

    private:
    AudioManager() = default;
    ~AudioManager() = default;
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
    MIX_Mixer* mixer = nullptr;
    bool initialized = false;

};

class RendererManager{
    public:
    static RendererManager& getInstance(){
        static RendererManager instance;
        if(!instance.initialized){
            instance.init();
            instance.initialized = true;
        }
        return instance;
    }
    void init(){
        window = SDL_CreateWindow(u8"lol_shooting",1280,720 ,SDL_WINDOW_RESIZABLE );
        renderer = SDL_CreateRenderer(window,NULL);
        camera = new Camera(renderer);
    }
    SDL_Window* getWindow() {
        return window;
    }
    SDL_Renderer*  getRenderer(){
        return renderer;
    }
    TTF_Font* getFont(){
        return font;
    }
    void setFont(const char* path,int size){
        font = TTF_OpenFont(path,size);
    }
    void barrel_fire_init(){
        animation_barrel_fire.set_loop(false);  //炮管开火
        animation_barrel_fire.set_interval(0.04f);
        animation_barrel_fire.set_center(center_barrel);
        animation_barrel_fire.add_frame(&atlas_barrel_fire);
        animation_barrel_fire.set_on_finished([&](){
            is_cool_down = true;
        });
        animation_barrel_fire.set_position({718,610});
    }

    void on_render(const Camera& camera);

    public:
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

    Animation animation_barrel_fire;
    bool is_cool_down = false;
    Camera* camera = nullptr;
    

    private:
    RendererManager() = default;
    ~RendererManager() = default;
    RendererManager(const RendererManager&) = delete;
    RendererManager& operator=(const RendererManager&) = delete;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    bool initialized = false;
    const SDL_FPoint center_barrel = {48,25};
    const Vector2 pos_battery = {640,600};
    const Vector2 pos_barrel = {717,610};

    //字体
    TTF_Font* font = nullptr;

};

class LogicManager {
    public:
    static LogicManager& getInstance(){
        static LogicManager instance;
        if(!instance.initialized){
            instance.init();
            instance.initialized = true;
        }
        return instance;
    }

    //基础配置
    void init(){
        RendererManager& rendererManager = RendererManager::getInstance();
        timer_generate.set_one_shot(false);
        timer_generate.set_wait_time(1.5f);
         timer_generate.set_on_timeout([&]()
        {
            for(int i = 0 ; i < num_per_gen ; i++){
                int val = rand() % 100;
                Monster* monster = nullptr;
                if(val < 50){
                     monster = new MonsterSlow();
                    monster->get_animation_run().add_frame(&rendererManager.atlas_monster_slow);
                }
                   
                else if(val < 80){
                    monster = new MonsterMedium();
                    monster->get_animation_run().add_frame(&rendererManager.atlas_monster_medium);
                }
                else {
                    monster = new MonsterFast();
                    monster->get_animation_run().add_frame(&rendererManager.atlas_monster_fast);
                }
                monster->get_animation_explosion().add_frame(&rendererManager.atlas_explosion);
                monster_list.push_back(monster);
            }
        }
        );
        timer_increase_num_per_gen.set_one_shot(false);
        timer_increase_num_per_gen.set_wait_time(8.0f);
        timer_increase_num_per_gen.set_on_timeout([&]()
            {
                num_per_gen += 1;
            });
    }

    std::vector<Monster*> get_monsterlist(){
        return monster_list;
    }

    std::vector<Bullet> get_bulletlist(){
        return bullet_list;
    }

    void logic_update_scene(float delta);

    Vector2 get_battery_pos(){
        return pos_battery;
    }

    int get_hp(){return hp;}
    int get_score(){return score;}
    //准星
    Vector2 pos_crosshair;
    //炮管方向
    double angle_barrel = 0;
    //炮管火焰
    bool is_fire_key_down = false;
    //结束标志
    bool is_quit = false;
    private:
    LogicManager() = default;
    ~LogicManager() = default;
    LogicManager(const LogicManager&)  = delete;
    LogicManager& operator=(const LogicManager&) = delete;
    bool initialized  =false;
    //计时器
    Timer timer_generate;
    Timer timer_increase_num_per_gen;
    //游戏元素更新
    std::vector<Monster*> monster_list;
    std::vector<Bullet> bullet_list;

    //坦克位置
    const Vector2 pos_battery = {640,600};
    const Vector2 pos_barrel = {717,610};

    int num_per_gen = 2;
    
    int score = 0;
    int hp = 10;

};

class GameManager{
    public:
    static GameManager& getInstance(){
        static GameManager instance;
        return instance;
    }
    AudioManager& audio() {
        return AudioManager::getInstance();
    }
    RendererManager& renderer(){
        return RendererManager::getInstance();
    }
    LogicManager& logic(){
        return LogicManager::getInstance();
    }
    

    
    //主函数
    void gameInit();
    void load_resources();
    void mainloop();
    private:
    GameManager() = default;
    ~GameManager() = default;
    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;
    
    

};
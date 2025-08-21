#include "gameManager.hpp"

void RendererManager::on_render(const Camera& camera){
    GameManager& gameManager = GameManager::getInstance();
    LogicManager& logicManager = gameManager.logic();

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
    for(Monster* monster : logicManager.get_monsterlist()){
         monster->on_render(camera);
    } 

    //绘制子弹
    for(Bullet& bullet : logicManager.get_bulletlist()){   
        Vector2 position = bullet.get_position();
        const SDL_FRect rect_bullet = {position.x - 4,position.y - 5,8,4};
        camera.render_texture(tex_bullet, nullptr, &rect_bullet, bullet.get_angle(), nullptr);
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
            camera.render_texture(tex_barrel_idle,nullptr,&rect_barrel,logicManager.angle_barrel,&center_barrel);
        else{
            animation_barrel_fire.set_rotation(logicManager.angle_barrel);
            animation_barrel_fire.on_render(camera);
        }
    }

    //绘制生命值
    {
        float width_heart,height_heart;
        SDL_GetTextureSize(tex_heart,&width_heart,&height_heart);
        for(int i = 0 ; i < logicManager.get_hp();i++){
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
        std::string str_score = "SCORE: "  + std::to_string(logicManager.get_score());
         SDL_Surface* suf_score_bg = TTF_RenderText_Blended(font, str_score.c_str(), str_score.size(), SDL_Color{55,55,55,255});
        SDL_Surface* suf_score_fg = TTF_RenderText_Blended(font, str_score.c_str(), str_score.size(), SDL_Color{255,255,255,255});

        SDL_Texture* tex_score_bg = SDL_CreateTextureFromSurface(renderer,suf_score_bg);
        SDL_Texture* tex_score_fg = SDL_CreateTextureFromSurface(renderer,suf_score_fg);
        SDL_FRect rect_dst_score = {static_cast<float>(1280 - suf_score_bg->w - 15),15,static_cast<float>(suf_score_bg->w),static_cast<float>(suf_score_bg->h)};
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
            logicManager.pos_crosshair.x - width_crosshair / 2.0f,
            logicManager.pos_crosshair.y - height_crosshair / 2.0f,
            static_cast<float>(width_crosshair),static_cast<float>(height_crosshair)
        };
        camera.render_texture(tex_crosshair,nullptr,&rect_crosshair,0,nullptr);
    }
}  

void LogicManager::logic_update_scene(float delta){
    GameManager& gameManager = GameManager::getInstance();
    AudioManager& audioMixer = gameManager.audio();
    RendererManager& rendererManager = gameManager.renderer();
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
            static const Vector2 size_monster ={40,50};
            if(pos_bullet.x >= pos_monster.x - size_monster.x / 2
            && pos_bullet.x <= pos_monster.x + size_monster.x /2
            && pos_bullet.y >= pos_monster.y - size_monster.y / 2 
            && pos_bullet.y <= pos_monster.y + size_monster.y / 2)
            {
                score += 1;
                bullet.on_hit();
                monster->set_alive(false);
               MIX_PlayAudio(audioMixer.getMixer(),audioMixer.sound_explosion);

            }
        }
        if(!monster->check_alive()) continue;

        //过线会减少生命值

        if(monster->get_position().y >= 720){
            monster->make_invalid();
            MIX_PlayAudio(audioMixer.getMixer(),audioMixer.sound_hurt);
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
    if(!rendererManager.is_cool_down){
        rendererManager.camera->shake(3.0f,0.1f);
        rendererManager.animation_barrel_fire.on_update(delta);
    }
    //处理开火瞬间逻辑
    if(rendererManager.is_cool_down && is_fire_key_down){
       rendererManager.animation_barrel_fire.reset();
        rendererManager.is_cool_down = false;

        static const float length_barrel = 105;
        static const Vector2 pos_barrel_center = {640,610};

        bullet_list.emplace_back(angle_barrel);
        Bullet& bullet = bullet_list.back();
        double angle_bullet = angle_barrel + (rand() % 30 - 15);
        double radians = angle_bullet * 3.14159265 / 180;
        Vector2 direction = {static_cast<float>(std::cos(radians)),static_cast<float>(std::sin(radians))};
        bullet.set_position(pos_barrel_center + direction * length_barrel);
        switch(rand() % 3){
            case 1 : MIX_PlayAudio(audioMixer.getMixer(),audioMixer.sound_fire_2); break;
            case 2 : MIX_PlayAudio(audioMixer.getMixer(),audioMixer.sound_fire_3); break;
            case 0 : MIX_PlayAudio(audioMixer.getMixer(),audioMixer.sound_fire_1); break;
        }
    }

    //更新摄像机位置
    rendererManager.camera->on_update(delta);

    //检查游戏是否结束
    if(hp <= 0){
        is_quit = true;
        MIX_PlayAudio(audioMixer.getMixer(),audioMixer.music_loss);
        std::string msg = u8"游戏最终得分：" + std::to_string(score);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, u8"游戏结束",msg.c_str(),rendererManager.getWindow());

    }
}


void GameManager::gameInit(){
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    MIX_Init();
    TTF_Init();
    SDL_HideCursor();
    load_resources();
   
};

void GameManager::load_resources(){
    GameManager& gameManager = GameManager::getInstance();
    AudioManager& audioMixer = gameManager.audio();
    RendererManager& rendererManager = gameManager.renderer();
    //视觉
    rendererManager.tex_heart = IMG_LoadTexture(rendererManager.getRenderer(),"resources\\heart.png");
    rendererManager.tex_bullet = IMG_LoadTexture(rendererManager.getRenderer(),"resources/bullet.png");
    rendererManager.tex_battery = IMG_LoadTexture(rendererManager.getRenderer(),"resources/battery.png");
    rendererManager.tex_crosshair = IMG_LoadTexture(rendererManager.getRenderer(),"resources/crosshair.png");
    rendererManager.tex_background = IMG_LoadTexture(rendererManager.getRenderer(),"resources/background.png");
    rendererManager.tex_barrel_idle = IMG_LoadTexture(rendererManager.getRenderer(),"resources/barrel_idle.png");
    //动画
    rendererManager.atlas_barrel_fire.load(rendererManager.getRenderer(),"resources/barrel_fire_%d.png",3);
    rendererManager.atlas_monster_fast.load(rendererManager.getRenderer(),"resources/monster_fast_%d.png",4);
    rendererManager.atlas_monster_medium.load(rendererManager.getRenderer(),"resources/monster_medium_%d.png",6);
    rendererManager.atlas_monster_slow.load(rendererManager.getRenderer(),"resources/monster_slow_%d.png",8);
    rendererManager.atlas_explosion.load(rendererManager.getRenderer(),"resources/explosion_%d.png",5);
    //音效
    audioMixer.music_bgm  = MIX_LoadAudio(audioMixer.getMixer(),"resources/bgm.mp3",false);
    audioMixer.music_loss =  MIX_LoadAudio(audioMixer.getMixer(),"resources/loss.mp3",false);
    audioMixer.sound_hurt = MIX_LoadAudio(audioMixer.getMixer(),"resources/hurt.wav",true);
    audioMixer.sound_fire_1 = MIX_LoadAudio(audioMixer.getMixer(),"resources/fire_1.wav",true);
    audioMixer.sound_fire_2 = MIX_LoadAudio(audioMixer.getMixer(),"resources/fire_2.wav",true);
    audioMixer.sound_fire_3 = MIX_LoadAudio(audioMixer.getMixer(),"resources/fire_3.wav",true);
    audioMixer.sound_explosion = MIX_LoadAudio(audioMixer.getMixer(),"resources/explosion.wav",true);
    //字体
    rendererManager.setFont("C:/Windows/Fonts/Arial.ttf",28);
    //开火动画
    rendererManager.barrel_fire_init();
    //播放背景音乐
    MIX_PlayAudio(audioMixer.getMixer(),audioMixer.music_bgm);
}


void GameManager::mainloop(){
    GameManager& gameManager = GameManager::getInstance();
    LogicManager& logicManager = gameManager.logic();
    RendererManager& rendererManager = gameManager.renderer();
    using namespace std::chrono;
    SDL_Event event;
    const nanoseconds frame_duration(1000000000/144);
    steady_clock::time_point last_tick = steady_clock::now();

  
    
    while(!logicManager.is_quit){
        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_EVENT_QUIT:
                    logicManager.is_quit = true;
                    break;
                case SDL_EVENT_MOUSE_MOTION:
                {
                    logicManager.pos_crosshair.x = static_cast<float>(event.motion.x);
                    logicManager.pos_crosshair.y = static_cast<float>(event.motion.y);
                    Vector2 direction = logicManager.pos_crosshair - logicManager.get_battery_pos();
                    logicManager.angle_barrel = std::atan2(direction.y,direction.x) * 180 / 3.14159265;
                    break;
                }
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    logicManager.is_fire_key_down = true;
                    break;
                case SDL_EVENT_MOUSE_BUTTON_UP:
                     logicManager.is_fire_key_down = false;
                    break;
            }
        }
        steady_clock::time_point frame_start = steady_clock::now();
        duration<float> delta = duration<float>(frame_start - last_tick);

        logicManager.logic_update_scene(delta.count());

        rendererManager.on_render(*rendererManager.camera);

        SDL_RenderPresent(rendererManager.getRenderer());

        last_tick = frame_start;
        nanoseconds sleep_duration = frame_duration - (steady_clock::now() - frame_start);
        if(sleep_duration > nanoseconds(0))
            std::this_thread::sleep_for(sleep_duration);
    }
}
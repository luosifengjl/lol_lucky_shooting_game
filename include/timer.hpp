#pragma once


#include <functional>

class Timer{
    public:
    Timer() = default;
    ~Timer() = default;

    void restart(){
        pass_time = 0;
        shotted  = false;
    }

    void set_wait_time(float time){
        wait_time = time;
    }

    void set_one_shot(bool flag){
        one_shot = flag;
    }

    void set_on_timeout(std::function<void()> func){
        on_timeout = func;
    }

    void pause(){
        paused = true;
    }

    void resume(){
        paused = false;
    }

    void on_update(float delta_time){
        if (paused) return;

        pass_time += delta_time;

        if (pass_time >= wait_time ) {
            bool can_shot = (!one_shot || (one_shot && !shotted));
            shotted = true;
            if (can_shot && on_timeout) {
                on_timeout();
            }
            pass_time  -= wait_time;
        }
    }
    private:
    float pass_time = 0.0f;  //0.0f明确指定为float类型 默认的浮点数字面量是double
    float wait_time = 0.0f;
    bool paused = false;
    bool shotted = false;
    bool one_shot = false;
    std::function<void()> on_timeout = nullptr;
};
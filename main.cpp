#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>
#include <string>

std::mutex mtx;
std::condition_variable cv;
std::atomic<bool> buttonPress = false;

enum class State
{
    GREEN,
    YELLOW,
    RED
};

void trafficLight(int green, int yellow, int red) 
{
    State state = State::RED;

    while(true)
    {
        std::unique_lock<std::mutex> lck(mtx);
        cv.wait(lck, []{ return buttonPress.load(); } );

        if (buttonPress)
        {
            state = State::RED;
            std::this_thread::sleep_for(std::chrono::seconds(10));
            buttonPress = false;
        } else {
            switch (state)
            {
            case State::GREEN:
                std::this_thread::sleep_for(std::chrono::seconds(green));
                state = State::YELLOW;
                break;
            case State::YELLOW:
                std::this_thread::sleep_for(std::chrono::seconds(yellow));
                state = State::RED;
                break;
            case State::RED:
                std::this_thread::sleep_for(std::chrono::seconds(red));
                state = State::GREEN;
                break;
            }
        }

        std::string strState;
        switch (state)
        {
        case State::GREEN:
            strState = "GREEN";
            break;
        case State::YELLOW:
            strState = "YELLOW";
            break;
        case State::RED:
            strState = "RED";
            break;
        }
        std::lock_guard<std::mutex> lck(mtx);
        std::cout << "Light is: " << strState << "\n";
    }
}

int main()
{
    return 0;
}
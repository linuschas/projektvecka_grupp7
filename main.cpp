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
std::atomic<bool> buttonPress(false);
std::atomic<bool> isRunning(true);

enum class State
{
    GREEN,
    YELLOW,
    RED
};

void buttonSimulator(const std::atomic_bool *isRunning, std::atomic_bool *buttonPress, std::mutex *mtx, std::condition_variable *cv);

int main() {
    int green = 10;
    int yellow = 3;
    int red = 7;

    std::thread tButton(buttonSimulator, &isRunning, &buttonPress, &mtx &cv);
    std::thread tTrafficLight(trafficLight, green, yellow, red);

    tButton.join();
    tTrafficLight.join();

    return 0;
}

void displayLight(State state)
{
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

void trafficLight(int green, int yellow, int red) 
{
    State state = State::RED;
    displayLight(state);

    while(isRunning)
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
        displayLight(state);
    }
}

void buttonSimulator(const std::atomic_bool *isRunning, std::atomic_bool *buttonPress, std::mutex *mtx, std::condition_variable *cv) {
    std::random_device rd;
    std::mt19937 gen(rd());

    while (*isRunning) {
        std::uniform_int_distribution<> dist(1, 10);

        int randNum = dist(gen);
        std::this_thread::sleep_for(std::chrono::seconds(randNum));

        mtx->lock();
        *buttonPress = true;
        cv->notify_all();
        mtx->unlock();
    }
}
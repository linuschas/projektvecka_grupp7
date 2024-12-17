#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#ifdef _WIN32
#    include <conio.h>
#else
#    include <termios.h>
#    include <unistd.h>
#endif

std::mutex mtx;
std::condition_variable cv;
std::atomic<bool> buttonPress(false);
std::atomic<bool> isRunning(true);

constexpr int GREEN_TIME  = 10;
constexpr int YELLOW_TIME = 3;
constexpr int RED_TIME    = 7;

enum class State
{
    GREEN,
    YELLOW,
    RED
};

void buttonSimulator(const std::atomic_bool* isRunning, std::atomic_bool* buttonPress,
                     std::mutex* mtx, std::condition_variable* cv);
void trafficLight(int green, int yellow, int red);
void keyboardHandler();

int main()
{
    std::thread tButton(buttonSimulator, &isRunning, &buttonPress, &mtx, &cv);
    std::thread tTrafficLight(trafficLight, GREEN_TIME, YELLOW_TIME, RED_TIME);
    std::thread tKeyboardHandler(keyboardHandler);

    tButton.join();
    tTrafficLight.join();
    tKeyboardHandler.join();

    return 0;
}

void sleepWithInterrupt(int seconds)
{
    for (int i = 0; i < seconds * 10; ++i)
    {
        if (!isRunning || buttonPress)
        {
            cv.notify_all();
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void keyboardHandler()
{
#ifdef _WIN32
    while (isRunning)
    {
        if (_kbhit())
        {
            char ch = _getch();
            if (ch == 'q')
            {
                isRunning = false;
                cv.notify_all();
            }
            else
            {
                std::lock_guard<std::mutex> lck(mtx);
                buttonPress = true;
                cv.notify_all();
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
#else
    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while (isRunning)
    {
        char ch = getchar();
        if (ch == 'q')
        {
            isRunning = false;
            cv.notify_all();
        }
        else
        {
            std::lock_guard<std::mutex> lck(mtx);
            buttonPress = true;
            cv.notify_all();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
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
    std::cout << "Traffic light is: " << strState << "\n";
}

void trafficLight(int green, int yellow, int red)
{
    State state = State::GREEN;
    // displayLight(state);

    while (isRunning)
    {
        bool buttonPressScope = false;
        {
            std::unique_lock<std::mutex> lck(mtx);
            cv.wait(lck, [] { return buttonPress.load() || isRunning; });

            if (buttonPress)
            {
                buttonPressScope = true;
                buttonPress      = false;
            }
        }

        if (!isRunning)
            break;

        if (buttonPressScope)
        {
            state       = State::GREEN;
            buttonPress = false;
            std::cout << "Button is pressed\n";
        }

        displayLight(state);

        switch (state)
        {
            case State::GREEN:
                sleepWithInterrupt(green);
                state = State::YELLOW;
                break;
            case State::YELLOW:
                sleepWithInterrupt(yellow);
                state = State::RED;
                break;
            case State::RED:
                sleepWithInterrupt(red);
                state = State::GREEN;
                break;
        }
    }
}

void buttonSimulator(const std::atomic_bool* isRunning, std::atomic_bool* buttonPress,
                     std::mutex* mtx, std::condition_variable* cv)
{
    std::random_device rd;
    std::mt19937 gen(rd());

    while (*isRunning)
    {
        std::uniform_int_distribution<> dist(20, 30);
        int randNum = dist(gen);

        // std::this_thread::sleep_for(std::chrono::seconds(randNum));
        sleepWithInterrupt(randNum);

        if (!isRunning)
            break;

        mtx->lock();
        *buttonPress = true;
        cv->notify_all();
        mtx->unlock();
    }
}
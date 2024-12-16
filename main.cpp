#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <random>
#include <conio.h>

std::mutex mtx;
std::condition_variable cv;
std::atomic<bool> buttonPress(false);
std::atomic<bool> isRunning(true);

void buttonSimulator(const std::atomic_bool *isRunning, std::atomic_bool *buttonPress, std::mutex *mtx, std::condition_variable *cv);
void takeInput();


int main() {
    std::thread tButton(buttonSimulator, &isRunning, &buttonPress, &mtx, &cv);
    std::thread tInput(takeInput);

    tButton.join();
    tInput.join();

    return 0;
}

void buttonSimulator(const std::atomic_bool *isRunning, std::atomic_bool *buttonPress, std::mutex *mtx, std::condition_variable *cv) {
    std::random_device rd;

    while (*isRunning) {
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(1, 10);

        int randNum = dist(gen);
        std::this_thread::sleep_for(std::chrono::seconds(randNum));

        mtx->lock();
        *buttonPress = true;
        cv->notify_all();
        mtx->unlock();
    }
}

void takeInput() {
    while (isRunning) {
        if (getch() == 'q') {
            mtx.lock();
            isRunning = false;
            mtx.unlock();
        }
    }
}
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <random>

std::mutex mtx;
std::condition_variable cv;
std::atomic<bool> buttonPress(false);

void buttonSimulator(const std::atomic_bool *isRunning, std::atomic_bool *buttonPress, std::mutex *mtx, std::condition_variable *cv);


int main() {

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
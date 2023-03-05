#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>

using namespace std::chrono_literals;

int main() {
    std::mutex mutex;
    std::condition_variable cv;
    bool is_ping = true; // globally shared state

    auto ping = [&]() {
        while (true) {
            std::unique_lock lock(mutex);
            cv.wait(lock,[&]() { return is_ping; });

            std::this_thread::sleep_for(1s);
            std::cout << "ping" << std::endl;

            is_ping = !is_ping;
            cv.notify_one();
        }
    };

    auto pong = [&]() {
        while (true) {
            std::unique_lock lock(mutex);
            cv.wait(lock, [&]() { return !is_ping; });

            std::this_thread::sleep_for(1s);
            std::cout << "pong" << std::endl;

            is_ping = !is_ping;
            cv.notify_one();
        }
    };

    std::thread ping_thread(ping);
    std::thread pong_thread(pong);
    ping_thread.join();
    pong_thread.join();
}

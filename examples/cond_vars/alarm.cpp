#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>

using namespace std::chrono_literals;

int main() {
    std::mutex mutex;
    std::condition_variable cv;
    bool time_for_breakfast = false; // globally shared state

    auto student = [&]() {
        {
            std::unique_lock lock(mutex);
            cv.wait(lock, [&]() { return time_for_breakfast; });
        }
        std::cout << "Time to make coffee!" << std::endl;
    };

    std::thread my_thread(student);
    std::this_thread::sleep_for(2s);

    {
        std::scoped_lock lock(mutex);
        time_for_breakfast = true;
    }

    cv.notify_one(); // ring the alarm clock
    my_thread.join(); // wait until breakfast is finished
}

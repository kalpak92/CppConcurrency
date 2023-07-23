#include <iostream>
#include <mutex>
#include <thread>

std::mutex mtx;

void workOnResource() {
    mtx.lock();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    mtx.unlock();

    std::cout << "Work done." << std::endl;
}

int main() {
    std::thread t1(workOnResource);
    std::thread t2(workOnResource);

    t1.join();
    t2.join();
}
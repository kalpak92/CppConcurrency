#include <iostream>
#include <chrono>
#include <mutex>
#include <thread>

struct CriticalData
{
    std::mutex mtx;
};

void deadlock(CriticalData& a, CriticalData& b)
{
    a.mtx.lock();

    std::cout << "Get the first Mutex" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));  // causes the deadlock

    b.mtx.lock();
    std::cout << "Get the second Mutex" << std::endl;

    a.mtx.unlock();
    b.mtx.unlock();
}

int main()
{
    CriticalData c1;
    CriticalData c2;

    std::thread t1([&]
    {
        deadlock(c1, c2);
    });

    std::thread t2([&]
    {
        deadlock(c2, c1);
    });

    t1.join();
    t2.join();
}
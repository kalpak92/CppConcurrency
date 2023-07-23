#include <iostream>
#include <atomic>
#include <thread>

class Spinlock {
    std::atomic_flag atomicFlag;

public:
    Spinlock(): atomicFlag(ATOMIC_FLAG_INIT){}

    void lock() {
        while (atomicFlag.test_and_set());
    }

    void unlock() {
        atomicFlag.clear();
    }
};

Spinlock spinLock;

void workOnResource() {
    spinLock.lock();

    // shared resource

    spinLock.unlock();
    std::cout << "Work done" << std::endl;
}

int main() {
    std::thread t1(workOnResource);
    std::thread t2(workOnResource);

    t1.join();
    t2.join();
}
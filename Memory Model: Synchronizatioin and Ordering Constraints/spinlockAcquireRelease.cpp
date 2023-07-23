#include <iostream>
#include <atomic>
#include <thread>

/**
 * Here, the heavyweight synchronization of the two  threads with sequential consistency is replaced by the lightweight and more performant acquire-release semantic.
 * The behaviour remains unchanged.
*/
class Spinlock
{
    public:
        Spinlock(): flag(ATOMIC_FLAG_INIT) {}

        void lock()
        {
            while(flag.test_and_set(std::memory_order_acquire));    // read-modify-write operation
        }

        void unlock()
        {
            flag.clear(std::memory_order_release);
        }

    private:
        std::atomic_flag flag;
};

Spinlock spin;

void workOnResource()
{
    spin.lock();
    // shared resource
    spin.unlock();
    std::cout << "Work Done." << std::endl;
}

int main()
{
    std::thread t1(workOnResource);
    std::thread t2(workOnResource);

    t1.join();
    t2.join();
}
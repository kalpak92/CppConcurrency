#include <iostream>
#include <chrono>
#include <mutex>
#include <thread>

using namespace std;

struct CriticalData
{
    mutex mtx;
};

void deadLock(CriticalData& a, CriticalData& b)
{
    cout << "Thread: " << this_thread::get_id() << " first mutex" << endl;
    this_thread::sleep_for(chrono::milliseconds(1));

    cout << "  Thread: " << this_thread::get_id() << " second mutex" <<  endl;
    cout << "    Thread: " << this_thread::get_id() << " get both mutex" << endl;

    std::scoped_lock(a.mtx, b.mtx);
    // do something with a and b
}

int main()
{
    CriticalData c1;
    CriticalData c2;

    thread t1([&]
    {
        deadLock(c1, c2);
    });

    thread t2([&]
    {
        deadLock(c2, c1);
    });

    t1.join();
    t2.join();
}
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
    lock_guard<mutex> guard1(a.mtx, adopt_lock);
    cout << "Thread: " << this_thread::get_id() << " first mutex" << endl;

    this_thread::sleep_for(chrono::milliseconds(1));

    // guard2 ownes mutex of CriticalData b
    lock_guard<mutex> guard2(b.mtx, adopt_lock);
    cout << "Thread: " << this_thread::get_id() << " second mutex" << endl;

    cout << "    Thread: " << this_thread::get_id() << " get both mutex" << endl;
    lock(guard1, guard2);   // locks the associated mutex
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
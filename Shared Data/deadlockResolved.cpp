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
    // Call to unique_lock with std::defer_lock does not lock the mutex automatically.
    // guard1 ownes mutex of CriticalData a
    unique_lock<mutex> guard1(a.mtx, defer_lock);
    cout << "Thread: " << this_thread::get_id() << " first mutex" << endl;

    this_thread::sleep_for(chrono::milliseconds(1));

    // guard2 ownes mutex of CriticalData b
    unique_lock<mutex> guard2(b.mtx, defer_lock);
    cout << "Thread: " << this_thread::get_id() << " second mutex" << endl;

    cout << "    Thread: " << this_thread::get_id() << " get both mutex" << endl;
    // std::lock tries to get all locks in one atomic step.
    // retries until it succeeds to get all of them.
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
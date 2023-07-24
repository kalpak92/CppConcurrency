#include <atomic>
#include <thread>
#include <iostream>
#include <string>

std::atomic<std::string*> ptr;
int data;
std::atomic<int> atoData;

void producer()
{
    std::string *p = new std::string("C++11");
    data = 2011;

    atoData.store(2014, std::memory_order_relaxed); // wont be reordered because of the release fence below
    
    std::atomic_thread_fence(std::memory_order_release);
    
    ptr.store(p, std::memory_order_relaxed);    // A release fence guarantees that the store operatioin cannot be moved before the fence
}

void consumer()
{
    std::string *p2;
    
    while(!(p2 = ptr.load(std::memory_order_relaxed))); // The acquire fence below guarantees that this operation will wont be moved after the fence

    std::atomic_thread_fence(std::memory_order_acquire);    // The Acquire fence synchronizes with Release fence.

    std::cout << "*p2: " << *p2 << std::endl;
    std::cout << "data: " << data << std::endl;
    std::cout << "atoData: " << atoData.load(std::memory_order_relaxed) << std::endl;   // Acquire fence guarantees these operation to not move before the fence.
}

int main()
{    
    std::thread t1(producer);
    std::thread t2(consumer);
    
    t1.join();
    t2.join();
    
    delete ptr;
}
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

std::vector<int> mySharedWork;
std::atomic<bool> dataProduced(false);
std::atomic<bool> dataConsumed(false);

void dataProducer()
{
    mySharedWork = {1, 0 ,3};
    dataProduced.store(true, std::memory_order_release);
}

void deliveryBoy()
{
    while(!dataProduced.load(std::memory_order_acquire));   // waits until dataProduced is True

    dataConsumed.store(true, std::memory_order_release);
}

void dataConsumer()
{
    while(!dataConsumed.load(std::memory_order_acquire));   // waits until dataConsumed is True

    mySharedWork[1] = 2;
}

// dataProducer --> sequenced before --> deliveryBoy --> sequenced before --> dataConsumer.
// The release operation synchronizes with an acquire operation on the same atomic variable.
// Each thread 'synchronizes with' each other because of the acquire release semantics of the atomic operations on the same atomic.
// The 'sequenced-before' and 'synchronizes-with' creates a HAPPENS-BEFORE relationship.

int main()
{
    
  std::cout << std::endl;

  std::thread t1(dataConsumer);
  std::thread t2(deliveryBoy);
  std::thread t3(dataProducer);

  t1.join();
  t2.join();
  t3.join();
  
  for (auto v: mySharedWork)
  {
    std::cout << v << " ";
  }
      
  std::cout << "\n\n";
  
}
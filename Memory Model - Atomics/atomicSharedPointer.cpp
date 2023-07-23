#include<iostream>
#include<atomic>
#include<thread>

int main()
{
    std::shared_ptr<int> ptr = std::make_shared<int>(2023);  
    for (auto i= 0; i<10; i++)
    {
        std::thread([&ptr]
	    {                         
            auto localPtr = std::make_shared<int>(2024);
            std::atomic_store(&ptr, localPtr);
        }).detach(); 
  }
}
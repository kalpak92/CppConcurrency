#include <iostream>
#include <map>
#include <string>
#include <chrono>
#include <shared_mutex>
#include <thread>

std::map<std::string, int> teleBook {
    {"Dijkstra", 1972}, 
    {"Scott", 1976}, 
    {"Ritchie", 1983}
};

std::shared_timed_mutex teleBookMtx;

void addToTeleBook(const std::string& name, int number)
{
    std::lock_guard<std::shared_timed_mutex> writeLock(teleBookMtx);    // lock_guard gives the exclusivity to write

    std::cout << "START UPDATE" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    teleBook[name] = number;
    std::cout << "END UPDATE" << std::endl;
}

void printNumber(const std::string& name)
{
    std::shared_lock<std::shared_timed_mutex> readLock(teleBookMtx);
    
    // reading an associative container can modify it when the element is not available.
    // Hence, explicity use only read operation.
    auto searchEntry = teleBook.find(name);
    if (searchEntry != teleBook.end())
    {
        std::cout << searchEntry->first << " : " << searchEntry->second << std::endl;
    }
    else
    {
        std::cout << name << " not found." << std::endl;
    }
}

int main()
{
    std::thread reader8([]{ printNumber("Bjarne"); });

    std::thread reader1([]{ printNumber("Scott"); });
    std::thread reader2([]{ printNumber("Ritchie"); });

    std::thread w1([]
    { 
        addToTeleBook("Scott",1968); 
    });
    
    std::thread reader3([]{ printNumber("Dijkstra"); });
    std::thread reader4([]{ printNumber("Scott"); });
    
    std::thread w2([]
    { 
        addToTeleBook("Bjarne",1965); 
    });
    
    std::thread reader5([]{ printNumber("Scott"); });
    std::thread reader6([]{ printNumber("Ritchie"); });
    std::thread reader7([]{ printNumber("Scott"); });
    // std::thread reader8([]{ printNumber("Bjarne"); });

    reader1.join();
    reader2.join();
    reader3.join();
    reader4.join();
    reader5.join();
    reader6.join();
    reader7.join();
    reader8.join();

    w1.join();
    w2.join(); 
}
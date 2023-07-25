#include <chrono>
#include <iostream>
#include <thread>
#include <mutex>

std::mutex coutMutex;

class Worker
{
    public:
        Worker(std::string n):name(n){};

        void operator()()
        {
            for(int i = 0; i <= 3; i++)
            {
                // Begin Work
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                // End Work

                coutMutex.lock();
                std::cout << name << ":" << "Work " << i << " done!!!" << std::endl;
                coutMutex.unlock();
            }
        }
    private:
        std::string name;
};

int main()
{
    std::cout << "Boss: Let's start working. \n\n";

    std::thread herb = std::thread(Worker("Herb"));
    std::thread andrei = std::thread(Worker("  Andrei"));
    std::thread scott = std::thread(Worker("    Scott"));
    std::thread bjarne = std::thread(Worker("       Bjarne"));
    std::thread bart = std::thread(Worker("             Bart"));
    std::thread jenne = std::thread(Worker("                Jenne"));

    herb.join();
    andrei.join();
    scott.join();
    bjarne.join();
    bart.join();
    jenne.join();

    std::cout << "\n Boss: Let's go home." << std::endl;
}
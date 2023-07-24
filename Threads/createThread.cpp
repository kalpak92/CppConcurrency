#include <iostream>
#include <thread>


void helloFunction()
{
    std::cout << "Hello from a function." << std::endl;
}

class HelloFunctionObject
{
    public:
        void operator()() const 
        {
            std::cout << "Hello from a function object." << std::endl;
        }
};

int main()
{
    std::thread t1(helloFunction);

    HelloFunctionObject obj1;
    std::thread t2(obj1);

    std::thread t3([]{
        std::cout << "Hello from a lambda function." << std::endl;
    });

    /**
     * The three threads are executed in an arbitrary order; even the three output operations can interleave. 
     * The creator of the child - ​the main thread in our case - is responsible for the lifetime of the child.
    */
   
    t1.join();
    t2.join();
    t3.join();
}
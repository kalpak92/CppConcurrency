# Thread Management

- Every C++ program has at least one thread which is started by the C++ runtime : the thread running `main()`.
- Additional threads can be created that have a different function as the entry point; which runs concurrently with each other.

## Launching a Thread

- Simple Case :
  - A simple task which is a plain, ordinary `void` returning function which takes no parameters.
  - Task can be a function object that takes additional parameters and performs a series of independent operations that are specified through some messaging system, whiile its running.

### Basic thread object

```C++
void do_some_work();
std::thread my_thread(do_some_work);
```

- `std::thread` works with any callable type.
  - Can be instance of a class with a function call operator.

```C++
class background_task
{
public:
    void operator()() const
    {
        do_something();
        do_something_else();
    }
};

background_task f;
std::thread my_thread(f);
```

### C++ Parse Vexing

If you pass a temporary instead of a named variable, the syntax can be the same as that of a function declaration, in which case the compiler interprets as a function declaration instead of a object definition.

```C++
std::thread my_thread(background_task());
```

This declares a thread function that takes a single parameter and returns a `std::thread` object, instead of launching a new thread.

To avoid this :

- Extra parenthesis to prevent interpretation as a function declaration.

    ```C++
    std::thread my_thread((background_task()));
    ```

- Use the new uniform initialization syntax with braces rather than parenthesis.

    ```C++
    std::thread my_thread{background_task()};
    ```

Additionally, to avoid this problem, one can use **lambda expression**. This allows writing of local function, capturing local variables and avoiding the need to pass additional arguments.

```c++
std::thread my_thread([]{
    do_something();
    do_something_else();
});
```

## Thread join/detach

Decision has to be made whether to wait for the thread to finish, or leave the thread to run on its own.

- The thread gets terminated by invoking `std::terminate()` in the destructor of `std::thread`.

> If you don't wait for the thread to finish, ensure that the data accessed by the thread is valid until the thread has finished using it.

```c++
struct func
    {
        int& i;
        func(int& i_):i(i_){}
        void operator()()
        {
            for(unsigned j=0;j<1000000;++j)
            {
                do_something(i);                        1
            }
        }
    };

    void oops()
    {
        int some_local_state=0;
        func my_func(some_local_state);
        std::thread my_thread(my_func);
        my_thread.detach();                             2
    }                                                   3
```

Here:

1. Potential access to dangling reference
2. Does not wait for the thread to finish
3. New thread might still be running.

### Waiting in exceptional circumstances

It is important that either `join()` or `detach()` is called before a `std::thread` object is destroyed.

It is important to carefully choose the point of `join()`; as it might be skipped if an exception is thrown after the thread has started but before the call to `join()`.

In general, if you are calling `join()` in a non-exceptional case, you also call `join()` in the presence of an exception to avoid accidental lifetime problems.

```C++
struct func
{
    int& i;
    func(int& i_):i(i_){}
    void operator()()
    {
        for(unsigned j=0;j<1000000;++j)
        {
            do_something(i);                        1
        }
    }
};

void f()
{
    int some_local_state=0;
    
    func my_func(some_local_state);
    std::thread t(my_func);

    try
    {
        do_something_in_current_thread();
    }
    catch(...)
    {
        t.join();
        throw;
    }
    t.join();
}
```

### Backgroud threads

Detached threads truly run in the background; ownership and control are passed over to the **C++ Runtime Library**, which ensures that the resources associated with the thread are correctly reclaimed when the thread exits

Calling detach() on a std::thread object leaves the thread to run in the background, with no direct means of communicating with it.

```C++
std::thread t(do_background_work);
t.detach();
assert(!t.joinable());
```

## Passing arguments to a thread function

Passing arguments to a callable object or function is fundamentally as sumple as passing additional arguments to `std::thread` constructor.

Important to note that  :

> By default, the arguments are `copied` into **internal storage**, where they can be  accessed by the  newly created thread of execution, and then passed to the callable object or function as `rvalues` as if they were temporaries.

### Function expects  a const reference

Consider

```C++
void f(int i,std::string const& s);
std::thread t(f,3,"hello");
```

Here, even though `f` takes a `std::string` as  the second parameter, thte string literal is passed as a `char const*` and converted to a `std::string` only in the context of the new thread.

```C++
void f(int i,std::string const& s);
void oops(int some_param)
{
    char buffer[1024];
    sprintf(buffer, "%i",some_param);

    std::thread t(f, 3, buffer);
    
    t.detach();
}
```

Here, it is a **pointer to a local variable `buffer`** that is passed through to the new thread, and the function `oops()` can possibly exit before the `buffer` has been converted to a `std::string` on the new thread.

Hence, leads to **UNDEFINED BEHAVIOR**.
In this case, the problem is that you were relying on the implicit conversion of the pointer to the buffer into the std::string object expected as a function parameter, but this conversion happens too late because the std::thread constructor copies the supplied values as is, without converting to the expected argument type.


Here, the solution is to explicitly cast it to `std::string`

```C++
void f(int i,std::string const& s);
void oops(int some_param)
{
    char buffer[1024];
    sprintf(buffer, "%i",some_param);

    std::thread t(f, 3, std::string(buffer));
    
    t.detach();
}
```


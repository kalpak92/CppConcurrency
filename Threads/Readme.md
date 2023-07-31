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

In this case, the problem is that you were relying on the implicit conversion of the pointer to the buffer into the std::string object expected as a function parameter, but this conversion happens too late because the `std::thread` constructor ***copies the supplied values as is, without converting to the expected argument type***.

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

### Function expects a non-const reference

Consider

```C++
void update_data_for_widget(widget_id w, widget_data& data);
void oops_again(widget_id w)
{
    widget_data data;

    std::thread t(update_data_for_widget, w, data);
    
    display_status();
    
    t.join();
    
    process_widget_data(data);
}
```

- Although the `update_data_for_widget` expects the second parameter to be passed by reference, the `std::thread` constructor does not know that.
- It is oblivious of the types of arguments expected by the function and blindly copies the supplied values.
- But the internal code passes copied arguments as `rvalues` in order to work with the move-only types.

> This will fail to compile because you cannot pass an `rvalue` to a function that expects a `non-const` reference.

To solve this, wrap the arguments in `std::ref`. This passes a reference instead of a `rvalue`.

```C++
std::thread t(update_data_for_widget, w, std::ref(data));
```

### Member function pointer as the function

You can pass a member function pointer as the function, provided you supply a suitable object pointer as the first argument.

```C++
class X
{
public:
    void do_lengthy_work();
};

X my_x;

std::thread t(&X::do_lengthy_work, &my_x);
```

This code will invoke `my_x.do_lengthy_work()` on the new thread because the address of `my_x` is supplied as the object pointer.

### Arguments cannot be COPIED, but MOVED

In this case, the data held within one object is transferred over to another, leaving the original object empty.

An example case study for this is : `std::unique_ptr`.

An `std::unique_ptr` provides automatic memory management for dynamically allocated objects. Only one `std::unique_ptr` instance can points to a given object at a time, and when that instance is destroyed, the pointed-to object is deleted.

The `move constructor` and the `move assignment operator` allows the ownership of an object to be transffered around between `std::unique_ptr` instances. Such transfer leaves the source object with a NULL pointer.

> Where the source object is temporary, the move is automatic; but where the source is a named value, the transfer must be requested directly by invoking `std::move()`.

```C++
void process_big_object(std::unique_ptr<big_object>);

std::unique_ptr<big_object> p(new big_object);

p->prepare_data(42);

std::thread t(process_big_object,std::move(p));
```

By specifying `std::move(p)` in the `std::thread` constructor, the ownership of `big_object` is transferred first into internal storage for the newly created thread, and then into `process_big_object`.

## Transferring ownership of  a thread

Suppose you want to write a function that creates a thread to run in the background, but passes ownership of the new thread back to the calling function rather than waiting for it to complete;

or

Create a thread and pass ownership in to some function that should wait for it to complete.

In either cases, we  need to transfer ownership from one place to another.

> This is where  the move support of `std::thread` comes in. `std::thread` is movable, not copyable.
> This means that t he  ownership of a particular thread of execution can be moved between `std::thread` instances.

```C++
void some_function();
void some_other_function();

std::thread t1(some_function);
std::thread t2 = std::move(t1);

t1 = std::thread(some_other_function);

std::thread t3;

t3 = std::move(t2);
t1 = std::move(t3);          1
```

First, a new thread is started and associated with t1. 
Ownership is then transferred over to t2 when t2 is constructed, by invoking std::move() to explicitly move ownership.
At this point, t1 no longer has an associated thread of execution; the thread running some_function is now associated with t2.

Then, a new thread is started and associated with a temporary std::thread object. 
The subsequent transfer of ownership into t1 doesn’t require a call to `std::move()` to explicitly move ownership, **because the owner is a temporary object—moving from temporaries is automatic and implicit.**

Until now :

- t3 is default-constructed, which means that it’s created without any associated thread of execution.
- Ownership of the thread currently associated with t2 is transferred into t3, again with an explicit call to `std::move()`, because t2 is a named object.
- t1 is associated with the thread running `some_other_function`, 
- t2 has no associated thread, and 
- t3 is associated with the thread running `some_function`.

> The final move transfers ownership of the thread running `some_function` back to t1 where it started. 
> But in this case t1 already had an associated thread (which was running `some_other_function`), so `std::terminate()` is called to terminate the program. This is done for consistency with the `std::thread` destructor.
> You must explicitly wait for a thread to complete or detach it before destruction, and the same applies to assignment: 
>> You can’t just drop a thread by assigning a new value to the `std::thread`` object that manages it.

### Returning a `std::thread` from a function

```C++
std::thread f()
{
    void some_function();
    return std::thread(some_function);
}

std::thread g()
{
    void some_other_function(int);
    std::thread t(some_other_function,42);
    return t;
}
```

A  function can accept an instance of `std::thread` by value as one of its paramenters.

```C++
void f(std::thread t);

void g()
{
    void some_function();
    
    f(std::thread(some_function));
    
    std::thread t(some_function);

    f(std::move(t));    // ownership is transferred to the function
}
```
# ThreadPool

## 项目描述
ThreadPool是一个高效的线程池实现，支持固定数量和动态增长的线程模式。它可以用于并发任务的管理和执行，提高程序的性能和资源利用率。线程池通过管理线程的创建、销毁和复用，避免了频繁创建和销毁线程带来的开销，从而提高了程序的执行效率。

## 特性
- 支持固定数量和动态增长的线程模式
- 任务队列管理，支持任务的提交和执行
- 信号量机制，确保任务的同步和通信
- 任务返回值管理，支持任意类型的返回值

## 安装说明
提供安装项目所需的步骤。例如：

### 克隆仓库
```bash
git clone https://github.com/yourusername/ThreadPool.git
cd ThreadPool
```

### 安装依赖
```bash
# 如果有依赖，请在这里列出安装依赖的命令
```

## 使用方法
说明如何使用你的项目。可以包括代码示例和命令行指令。

### 示例代码
```cpp file test.cpp
#include <iostream>
#include <chrono>
#include <thread>
using namespace std;

#include "threadpool.h"

using uLong = unsigned long long;

class MyTask : public Task
{
public:
    MyTask(int begin, int end)
        : begin_(begin)
        , end_(end)
    {}
    Any run()
    {
        std::cout << "tid:" << std::this_thread::get_id()
            << "begin!" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
        uLong sum = 0;
        for (uLong i = begin_; i <= end_; i++)
            sum += i;
        std::cout << "tid:" << std::this_thread::get_id()
            << "end!" << std::endl;

        return sum;
    }

private:
    int begin_;
    int end_;
};

int main()
{
    {
        ThreadPool pool;
        pool.setMode(PoolMode::MODE_CACHED);
        pool.start(2);

        Result res1 = pool.submitTask(std::make_shared<MyTask>(1, 100000000));
        Result res2 = pool.submitTask(std::make_shared<MyTask>(100000001, 200000000));
        pool.submitTask(std::make_shared<MyTask>(100000001, 200000000));
        pool.submitTask(std::make_shared<MyTask>(100000001, 200000000));
        pool.submitTask(std::make_shared<MyTask>(100000001, 200000000));

        //uLong sum1 = res1.get().cast_<uLong>();
        //cout << sum1 << endl;
    }

    cout << "main over!" << endl;
    getchar();
}
```

### 命令行指令
```bash
# 编译和运行项目
g++ -std=c++11 -pthread threadpool.cpp test.cpp -o threadpool
./threadpool
```

## 类和函数说明
### Any类
`Any`类用于存储任意类型的数据，支持任意类型的数据存储和提取。

```cpp file threadpool.h
class Any
{
public:
    Any() = default;
    ~Any() = default;
    Any(const Any&) = delete;
    Any& operator=(const Any&) = delete;
    Any(Any&&) = default;
    Any& operator=(Any&&) = default;

    template<typename T>
    Any(T data) : base_(std::make_unique<Derive<T>>(data))
    {}

    template<typename T>
    T cast_()
    {
        Derive<T>* pd = dynamic_cast<Derive<T>*>(base_.get());
        if (pd == nullptr)
        {
            throw "type is unmatch!";
        }
        return pd->data_;
    }
private:
    class Base
    {
    public:
        virtual ~Base() = default;
    };

    template<typename T>
    class Derive : public Base
    {
    public:
        Derive(T data) : data_(data)
        {}
        T data_;
    };

private:
    std::unique_ptr<Base> base_;
};
```

### Semaphore类
`Semaphore`类用于实现信号量机制，用于线程间的同步和通信。

```cpp file threadpool.h
class Semaphore
{
public:
    Semaphore(int limit = 0)
        :resLimit_(limit)
    {}
    ~Semaphore() = default;

    void wait()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        cond_.wait(lock, [&]()->bool {return resLimit_ > 0; });
        resLimit_--;
    }

    void post()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        resLimit_++;
        cond_.notify_all();
    }
private:
    int resLimit_;
    std::mutex mtx_;
    std::condition_variable cond_;
};
```

### Task类
`Task`类是任务的抽象基类，用户可以继承该类实现自定义任务。

```cpp file threadpool.h
class Task
{
public:
    Task();
    ~Task() = default;
    void exec();
    void setResult(Result* res);

    virtual Any run() = 0;

private:
    Result* result_;
};
```

### Result类
`Result`类用于存储任务的返回值，并提供获取返回值的方法。

```cpp file threadpool.h
class Result
{
public:
    Result(std::shared_ptr<Task> task, bool isValid = true);
    ~Result() = default;

    void setVal(Any any);
    Any get();
private:
    Any any_;
    Semaphore sem_;
    std::shared_ptr<Task> task_;
    std::atomic_bool isValid_;
};
```

### ThreadPool类
`ThreadPool`类是线程池的核心类，负责管理线程和任务队列。

```cpp file threadpool.h
class ThreadPool
{
public:
    ThreadPool();
    ~ThreadPool();

    void setMode(PoolMode mode);
    void setTaskQueMaxThreshHold(int threshhold);
    void setThreadSizeThreshHold(int threshhold);
    Result submitTask(std::shared_ptr<Task> sp);
    void start(int initThreadSize = std::thread::hardware_concurrency());

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

private:
    void threadFunc(int threadid);
    bool checkRunningState() const;

private:
    std::unordered_map<int, std::unique_ptr<Thread>> threads_;
    int initThreadSize_;
    int threadSizeThreshHold_;
    std::atomic_int curThreadSize_;
    std::atomic_int idleThreadSize_;
    std::queue<std::shared_ptr<Task>> taskQue_;
    std::atomic_int taskSize_;
    int taskQueMaxThreshHold_;
    std::mutex taskQueMtx_;
    std::condition_variable notFull_;
    std::condition_variable notEmpty_;
    std::condition_variable exitCond_;
    PoolMode poolMode_;
    std::atomic_bool isPoolRunning_;
};
```

## 贡献指南
说明如何为项目做出贡献。可以包括如何报告问题、如何提交拉取请求等。

### 报告问题
如果你发现了问题，请在GitHub上创建一个新的issue。

### 提交拉取请求
1. 克隆仓库并创建一个新的分支。
2. 进行你的更改并提交。
3. 创建一个拉取请求。

## 许可证
说明项目的许可证类型。例如：

此项目使用MIT许可证。详情请参阅[LICENSE](LICENSE)文件。

## 联系方式
提供你的联系方式，以便其他人可以与你联系。

- 邮箱：your.email@example.com
- GitHub：[yourusername](https://github.com/yourusername)

## 鸣谢
感谢所有为项目做出贡献的人和资源。
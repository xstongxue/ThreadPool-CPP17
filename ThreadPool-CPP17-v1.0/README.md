# ThreadPool-CPP17-v1.0

## 项目描述
ThreadPool-CPP17-v1.0是一个高效的线程池实现，支持固定数量和动态增长的线程模式。它可以用于并发任务的管理和执行，提高程序的性能和资源利用率。线程池通过管理线程的创建、销毁和复用，避免了频繁创建和销毁线程带来的开销，从而提高了程序的执行效率。

## 特性
- 支持固定数量和动态增长的线程模式
- 任务队列管理，支持任务的提交和执行
- 使用C++11的`std::future`和`std::packaged_task`代替自定义的`Result`类，简化了代码

## 安装说明
提供安装项目所需的步骤。例如：

### 克隆仓库
```bash
git clone https://github.com/yourusername/ThreadPool-CPP17-v1.0.git
cd ThreadPool-CPP17-v1.0
```

### 安装依赖
```bash
# 如果有依赖，请在这里列出安装依赖的命令
```

## 使用方法
说明如何使用你的项目。可以包括代码示例和命令行指令。

### 示例代码
```cpp file run.cpp
#include <iostream>
#include <functional>
#include <thread>
#include <future>
#include <chrono>
using namespace std;

#include "threadpool.h"

int sum1(int a, int b)
{
    this_thread::sleep_for(chrono::seconds(2));
    return a + b;
}

int sum2(int a, int b, int c)
{
    this_thread::sleep_for(chrono::seconds(2));
    return a + b + c;
}

int main()
{
    ThreadPool pool;
    pool.start(2);

    future<int> r1 = pool.submitTask(sum1, 1, 2);
    future<int> r2 = pool.submitTask(sum2, 1, 2, 3);
    future<int> r3 = pool.submitTask([](int b, int e)->int {
        int sum = 0;
        for (int i = b; i <= e; i++)
            sum += i;
        return sum;
    }, 1, 100);

    cout << r1.get() << endl;
    cout << r2.get() << endl;
    cout << r3.get() << endl;
}
```

### 命令行指令
```bash
# 编译和运行项目
g++ -std=c++11 -pthread threadpool.h run.cpp -o threadpool
./threadpool
```

## 接口描述

### ThreadPool类
`ThreadPool`类是线程池的核心类，负责管理线程和任务队列。

#### 构造函数
```cpp
ThreadPool();
```
初始化线程池对象。

#### 析构函数
```cpp
~ThreadPool();
```
销毁线程池对象，确保所有线程资源被正确回收。

#### 设置线程池模式
```cpp
void setMode(PoolMode mode);
```
设置线程池的工作模式，支持固定数量和动态增长的线程模式。

#### 设置任务队列上限阈值
```cpp
void setTaskQueMaxThreshHold(int threshhold);
```
设置任务队列的上限阈值。

#### 设置线程数量上限阈值
```cpp
void setThreadSizeThreshHold(int threshhold);
```
设置线程数量的上限阈值，仅在动态增长模式下有效。

#### 提交任务
```cpp
template<typename Func, typename... Args>
auto submitTask(Func&& func, Args&&... args) -> std::future<decltype(func(args...))>;
```
提交任务到线程池，支持任意任务函数和参数。返回一个`std::future`对象，用于获取任务的返回值。

#### 启动线程池
```cpp
void start(int initThreadSize = std::thread::hardware_concurrency());
```
启动线程池，初始化指定数量的线程。

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
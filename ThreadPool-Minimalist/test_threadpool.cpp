// test_threadpool.cpp
#include "threadpool_mini.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <future>
#include <cassert>

// 定义一个简单的任务类
class SimpleTask : public Task {
public:
    SimpleTask(int id) : id(id) {}

    void execute() override {
        std::cout << "Task " << id << " is running on thread " << std::this_thread::get_id() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1)); // 模拟任务执行时间
        std::cout << "Task " << id << " completed" << std::endl;
    }

private:
    int id;
};

// 定义一个任务类，用于测试异常情况
class ExceptionTask : public Task {
public:
    void execute() override {
        throw std::runtime_error("Task threw an exception");
    }
};

void testThreadPool() {
    // 创建一个包含4个线程的线程池
    ThreadPool pool(4);

    // 提交多个任务到线程池
    std::vector<std::future<void>> futures;
    for (int i = 0; i < 10; ++i) {
        auto task = std::make_shared<SimpleTask>(i);
        futures.push_back(pool.submitTask(task));
    }

    // 等待所有任务完成
    for (auto& future : futures) {
        future.get();
    }

    std::cout << "All tasks completed." << std::endl;

    // 测试异常情况
    try {
        auto exceptionTask = std::make_shared<ExceptionTask>();
        auto future = pool.submitTask(exceptionTask);
        future.get(); // 这应该会抛出异常
    } catch (const std::exception& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }

    // 测试线程池的析构函数
    {
        ThreadPool localPool(2);
        auto task = std::make_shared<SimpleTask>(100);
        localPool.submitTask(task);
    }
    std::cout << "Local thread pool destroyed." << std::endl;
}

int main() {
    testThreadPool();
    return 0;
}
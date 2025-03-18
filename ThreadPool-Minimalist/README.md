# ThreadPool-Minimalist

## 项目描述

`ThreadPool-Minimalist` 是一个简单的线程池实现，使用C++17标准库。它允许你提交任务到线程池中，并由线程池管理的线程来执行这些任务。

## 安装说明

1. 确保你的编译器支持C++17标准。
2. 将 `threadpool_mini.hpp` 和 `threadpool_mini.cpp` 文件添加到你的项目中。

## 使用说明

### 定义任务

首先，你需要定义一个继承自 `Task` 类的任务类，并实现 `execute` 方法。

```cpp
class MyTask : public Task {
public:
    void execute() override {
        // 任务执行的代码
    }
};
```

### 创建线程池

然后，你可以创建一个 `ThreadPool` 对象，并提交任务到线程池中。

```cpp
int main() {
    ThreadPool pool(4); // 创建一个包含4个线程的线程池

    auto task = std::make_shared<MyTask>();
    pool.submitTask(task);

    // 等待任务完成
    task->get_future().get();

    return 0;
}
```

## 测试

我们提供了一个测试文件 `test_threadpool.cpp` 来验证 `ThreadPool` 的功能。你可以按照以下步骤进行测试：

1. 确保你已经安装了C++17兼容的编译器。
2. 使用以下命令编译测试程序：

```sh
g++ -std=c++17 -o test_threadpool test_threadpool.cpp threadpool_mini.cpp -pthread
```

3. 运行测试程序：

```sh
./test_threadpool
```

测试程序将展示线程池的工作原理，包括任务的分发、异常处理和线程池的清理。

## 贡献指南

欢迎贡献！请先阅读我们的 [贡献指南](CONTRIBUTING.md)。

## 许可证

本项目采用 MIT 许可证。详情请见 [LICENSE](LICENSE) 文件。

## 联系方式

如果你有任何问题或建议，请通过 [GitHub Issues](https://github.com/yourusername/ThreadPool-Minimalist/issues) 联系我们。
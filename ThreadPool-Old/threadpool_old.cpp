#include "threadpool_old.h"

#include <functional>
#include <thread>
#include <iostream>

const int TASK_MAX_THRESHHOLD = INT32_MAX;
const int THREAD_MAX_THRESHHOLD = 1024;
const int THREAD_MAX_IDLE_TIME = 60; // 单位：秒

// 线程池构造
ThreadPool::ThreadPool()
	: m_init_thread_size(0)
	, m_task_size(0)
	, m_idle_thread_size(0)
	, m_cur_thread_size(0)
	, m_task_que_max_thresh_hold(TASK_MAX_THRESHHOLD)
	, m_thread_size_thresh_hold(THREAD_MAX_THRESHHOLD)
	, m_pool_mode(PoolMode::MODE_FIXED)
	, m_is_pool_running(false)
{}

// 线程池析构
ThreadPool::~ThreadPool()
{
	m_is_pool_running = false;
	
	// 等待线程池里面所有的线程返回  有两种状态：阻塞 & 正在执行任务中
	std::unique_lock<std::mutex> lock(m_task_que_mtx);
	m_not_empty.notify_all();
	m_exit_cond.wait(lock, [&]()->bool {return m_threads.size() == 0; });
}

// 设置线程池的工作模式
void ThreadPool::setMode(PoolMode mode)
{
	if (checkRunningState())
		return;
	m_pool_mode = mode;
}

// 设置task任务队列上线阈值
void ThreadPool::setTaskQueMaxThreshHold(int threshhold)
{
	if (checkRunningState())
		return;
	m_task_que_max_thresh_hold = threshhold;
}

// 设置线程池cached模式下线程阈值
void ThreadPool::setThreadSizeThreshHold(int threshhold)
{
	if (checkRunningState())
		return;
	
	if (m_pool_mode == PoolMode::MODE_CACHED)
	{
		m_thread_size_thresh_hold = threshhold;
	}
}

// 开启线程池
void ThreadPool::start(int init_thread_size)
{
	// 设置线程池的运行状态
	m_is_pool_running = true;

	// 记录初始线程个数
	m_init_thread_size = init_thread_size;
	m_cur_thread_size = init_thread_size;

	// 创建线程对象
	for (int i = 0; i < m_init_thread_size; i++) {
		// 创建thread线程对象的时候，绑定线程成员函数threadFunc，并传递当前线程的ID，this指向ThreadPool对象
		// Thread 构造函数 需要 一个可调用对象（函数、lambda 或 std::bind 绑定的函数） 作为线程执行体
		auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
		int threadId = ptr->getId();
		// 将创建的Thread对象存储到m_threads容器中，使用线程ID作为键
		m_threads.emplace(threadId, std::move(ptr));
		// m_threads.emplace_back(std::move(ptr)); // 备选方案，直接将Thread对象添加到容器末尾
	}

	// 启动所有线程  std::vector<Thread*> m_threads;
	for (int i = 0; i < m_init_thread_size; i++)
	{
		m_threads[i]->start(); // 需要去执行一个线程函数
		m_idle_thread_size++;    // 记录初始空闲线程的数量
	}
}

// 定义线程函数   线程池的所有线程从任务队列里面消费任务
void ThreadPool::threadFunc(int threadid)  // 线程函数返回，相应的线程也就结束了
{
	auto lastTime = std::chrono::high_resolution_clock().now();

	// 所有任务必须执行完成，线程池才可以回收所有线程资源
	for (;;)
	{
		std::shared_ptr<Task> task;
		{
			// 先获取锁
			std::unique_lock<std::mutex> lock(m_task_que_mtx);

			std::cout << "tid:" << std::this_thread::get_id()
				<< "尝试获取任务..." << std::endl;

			// cached模式下，有可能已经创建了很多的线程，但是空闲时间超过60s，应该把多余的线程
			// 结束回收掉（超过init_thread_size_数量的线程要进行回收）
			// 当前时间 - 上一次线程执行的时间 > 60s
			
			// 每一秒中返回一次   怎么区分：超时返回？还是有任务待执行返回
			// 锁 + 双重判断
			while (m_task_que.size() == 0)
			{
				// 线程池要结束，回收线程资源
				if (!m_is_pool_running)
				{
					m_threads.erase(threadid); // std::this_thread::getid()
					std::cout << "threadid:" << std::this_thread::get_id() << " exit!"
						<< std::endl;
					m_exit_cond.notify_all();
					return; // 线程函数结束，线程结束
				}

				if (m_pool_mode == PoolMode::MODE_CACHED)
				{
					// 条件变量，超时返回了
					if (std::cv_status::timeout ==
						m_not_empty.wait_for(lock, std::chrono::seconds(1)))
					{
						auto now = std::chrono::high_resolution_clock().now();
						auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
						if (dur.count() >= THREAD_MAX_IDLE_TIME
							&& m_cur_thread_size > m_init_thread_size)
						{
							// 开始回收当前线程
							// 记录线程数量的相关变量的值修改
							// 把线程对象从线程列表容器中删除   没有办法 threadFunc《=》thread对象
							// threadid => thread对象 => 删除
							m_threads.erase(threadid); // std::this_thread::getid()
							m_cur_thread_size--;
							m_idle_thread_size--;

							std::cout << "threadid:" << std::this_thread::get_id() << " exit!"
								<< std::endl;
							return;
						}
					}
				}
				else
				{
					// 等待 m_not_empty 条件
					m_not_empty.wait(lock);
				}

				//if (!m_is_pool_running)
				//{
				//	m_threads.erase(threadid); // std::this_thread::getid()
				//	std::cout << "threadid:" << std::this_thread::get_id() << " exit!"
				//		<< std::endl;
				//	m_exit_cond.notify_all();
				//	return; // 结束线程函数，就是结束当前线程了!
				//}
			}

			m_idle_thread_size--;

			std::cout << "tid:" << std::this_thread::get_id()
				<< "获取任务成功..." << std::endl;

			// 从任务队列种取一个任务出来
			task = m_task_que.front();
			m_task_que.pop();
			m_task_size--;

			// 如果依然有剩余任务，继续通知其它得线程执行任务
			if (m_task_que.size() > 0)
			{
				m_not_empty.notify_all();
			}

			// 取出一个任务，进行通知，通知可以继续提交生产任务
			m_not_full.notify_all();
		} // 就应该把锁释放掉
		
		// 当前线程负责执行这个任务
		if (task != nullptr)
		{
			// task->run(); // 执行任务；把任务的返回值setVal方法给到Result
			task->exec();
		}
		
		m_idle_thread_size++;
		lastTime = std::chrono::high_resolution_clock().now(); // 更新线程执行完任务的时间
	}
}

// 给线程池提交任务    用户调用该接口，传入任务对象，生产任务
Result ThreadPool::submitTask(std::shared_ptr<Task> sp)
{
	// 获取锁
	std::unique_lock<std::mutex> lock(m_task_que_mtx);

	// 线程的通信  等待任务队列有空余   wait   wait_for   wait_until
	// 用户提交任务，最长不能阻塞超过1s，否则判断提交任务失败，返回
	if (!m_not_full.wait_for(lock, std::chrono::seconds(1),
		[&]()->bool { return m_task_que.size() < (size_t)m_task_que_max_thresh_hold; }))
	{
		// 表示notFull_等待1s种，条件依然没有满足
		std::cerr << "task queue is full, submit task fail." << std::endl;
		// return task->getResult();  // Task  Result   线程执行完task，task对象就被析构掉了
		return Result(sp, false);
	}

	// 如果有空余，把任务放入任务队列中
	m_task_que.emplace(std::move(sp));
	m_task_size++;

	// 因为新放了任务，任务队列肯定不空了，在notEmpty_上进行通知，赶快分配线程执行任务
	m_not_empty.notify_all();

	// cached模式 任务处理比较紧急 场景：小而快的任务 需要根据任务数量和空闲线程的数量，判断是否需要创建新的线程出来
	if (m_pool_mode == PoolMode::MODE_CACHED
		&& m_task_size > m_idle_thread_size
		&& m_cur_thread_size < m_thread_size_thresh_hold)
	{
		std::cout << ">>> create new thread..." << std::endl;

		// 创建新的线程对象
		auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
		int threadId = ptr->getId();
		m_threads.emplace(threadId, std::move(ptr));
		// 启动线程
		m_threads[threadId]->start(); 
		// 修改线程个数相关的变量
		m_cur_thread_size++;
		m_idle_thread_size++;
	}

	// 返回任务的Result对象
	return Result(sp);
	// return task->getResult();
}

bool ThreadPool::checkRunningState() const
{
	return m_is_pool_running;
}

////////////////  线程方法实现
int Thread::m_generate_id = 0;

// 线程构造
Thread::Thread(ThreadFunc func)
	: m_func(func)
	, m_thread_id(m_generate_id++)
{}

// 线程析构
Thread::~Thread() {}

// 启动线程
void Thread::start()
{
	// 创建一个线程来执行一个线程函数 pthread_create
	// 注释：创建一个新的 std::thread 线程，它会立即执行 m_func(m_thread_id)
	std::thread t(m_func, m_thread_id);  // C++11来说 线程对象t 和线程函数func_

	// 注释：将线程设置为分离线程（detached thread），让它在后台独立运行，主线程无需等待它完成（防止阻塞）
	t.detach(); // 类似POSIX线程（pthreads）：pthread_detach  pthread_t设置成分离线程
}

int Thread::getId()const
{
	return m_thread_id;
}


/////////////////  Task方法实现
Task::Task()
	: m_result(nullptr)
{}

void Task::exec()
{
	if (m_result != nullptr)
	{
		m_result->setVal(run()); // 这里发生多态调用
	}
}

// 指针判空：防止在linux下报错
void Task::setResult(Result* res)
{
	if(res != nullptr) // 使用前检查指针的有效性
	{  
        m_result = res;
    }
}

/////////////////   Result方法的实现
Result::Result(std::shared_ptr<Task> task, bool isValid)
	: m_is_valid(isValid)
	, m_task_obj(task)
{
	// 指针判空：防止在linux下报错
    if (task != nullptr) // 使用前检查指针的有效性
	{   
        task->setResult(this);
    }
}

Any Result::get() // 用户调用的
{
	if (!m_is_valid)
	{
		return "";
	}
	m_sem.wait(); // task任务如果没有执行完，这里会阻塞用户的线程
	return std::move(m_any);
}

void Result::setVal(Any any)  // 谁调用的呢？？？ 这里发生多态调用
{
	// 存储task的返回值
	this->m_any = std::move(any);
	m_sem.post(); // 已经获取的任务的返回值，增加信号量资源
}
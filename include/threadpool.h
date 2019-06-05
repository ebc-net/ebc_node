#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

namespace NET
{
#define MAX_THREADPOOL_NUM 256

//线程池,不是固定参数的, 无参数数量限制
class threadpool
{
    using Task = std::function<void()>;
    std::vector<std::thread> pool;//线程池
    std::queue<Task> tasks;//队列任务
    std::mutex m_lock;//同步 互斥锁
    std::condition_variable cond;//条件变量，阻塞
    std::atomic<bool> run{true};//线程池是否执行 可实现数据结构的无锁设计
    std::atomic<int> idleTrdNum{0};//空闲线程数量

public:
    inline threadpool(unsigned short size = 4){addThread(size);}
    inline ~threadpool()
    {
        run = false;
        cond.notify_all();// 唤醒所有线程执行
        for(auto& thread:pool)
        {
//            thread.detach();// 让线程“自生自灭”
            if(thread.joinable())
                thread.join();// 等待任务结束，前提：线程一定会执行完
        }
    }

public:
    template<class F, class...Args>
    auto commit(F&& f, Args&&... args) ->std::future<decltype (f(args...))>
    {
        if(!run)
            throw  std::runtime_error("commit on threadpool is stopped.");
        using RetType = decltype (f(args...));
        auto task = std::make_shared<std::packaged_task<RetType()>>(
                    std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        std::future<RetType> future = task->get_future();
        {
            std::lock_guard<std::mutex> lock{m_lock};
            tasks.emplace([task](){
                (*task)();
            });
        }
        cond.notify_one();//唤醒一个线程执行,唤醒处于wait中的其中一个条件变量
        return future;
    }

    int idleCount(){return idleTrdNum;}//空闲线程数量
    int thrdCount(){return pool.size();}//线程数量

    void addThread(unsigned short size)//添加指定数量的线程
    {
        for (; pool.size() < MAX_THREADPOOL_NUM && size > 0; --size)
        {
            //初始化线程数量
            pool.emplace_back([this]//工作线程函数       emplace_back比push_back效率高
            {
                while(run)
                {
                    Task task; //获取一个执行的 task
                    {//大括号作用：临时变量的生存期，即控制lock时间
                        //unique_lock相比lock_guard的好处是：可以随时 unlock()和 lock()
                        std::unique_lock<std::mutex> lock{m_lock};
                        this->cond.wait(lock,[this]// wait 直到有task
                        {
                            return  !run || !this->tasks.empty();
                        });
                        if(!run && tasks.empty())
                            return ;
                        task = std::move(tasks.front());// 取一个 task
                        tasks.pop();
                    }
                    idleTrdNum--;
                    task();
                    idleTrdNum++;
                }
            });
            idleTrdNum++;
        }
    }
};
}

#endif // THREADPOOL_H


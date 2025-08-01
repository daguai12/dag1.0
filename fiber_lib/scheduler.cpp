#include <iostream>
#include <assert.h>


#include "scheduler.h"
#include "logger.h"
#include "hook.h"
#include "./utils/util.h"

namespace dag{

static Logger::ptr g_logger = DAG_LOG_ROOT();

static thread_local Scheduler* t_scheduler = nullptr;

Scheduler* Scheduler::GetThis()
{
	return t_scheduler;
}

void Scheduler::SetThis()
{
	t_scheduler = this;
}

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string &name)
    : m_useCaller(use_caller)
    , m_name(name)
{
	assert(threads>0 && Scheduler::GetThis()==nullptr);


    Thread::SetName(m_name);

	// 使用主线程当作工作线程
	if(use_caller)
	{
		threads --;

		// 创建主协程
		Fiber::GetThis();

        t_scheduler = this;

		// 创建调度协程
		m_schedulerFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, false)); // false -> 该调度协程退出后将返回主协程
		Fiber::SetSchedulerFiber(m_schedulerFiber.get());
		
		m_rootThread = getThreadId();
		m_threadIds.push_back(m_rootThread);
	}

	m_threadCount = threads;
}

Scheduler::~Scheduler()
{
	assert(stopping()==true);
	if (GetThis() == this) 
	{
        t_scheduler = nullptr;
    }
}

void Scheduler::start()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if(m_stopping)
	{
		std::cerr << "Scheduler is stopped" << std::endl;
		return;
	}

	assert(m_threads.empty());
	m_threads.resize(m_threadCount);
	for(size_t i=0;i<m_threadCount;i++)
	{
		m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this), m_name + "_" + std::to_string(i)));
		m_threadIds.push_back(m_threads[i]->getId());
	}
}

void Scheduler::run()
{
    DAG_LOG_DEBUG(g_logger) << m_name << " run";
	int thread_id = getThreadId();
	
	set_hook_enable(true);

	SetThis();

	// 运行在新创建的线程 -> 需要创建主协程
	if(thread_id != m_rootThread)
	{
		Fiber::GetThis();
	}

	std::shared_ptr<Fiber> idle_fiber = std::make_shared<Fiber>(std::bind(&Scheduler::idle, this));
	SchedulerTask task;
	
	while(true)
	{
		task.reset();
		bool tickle_me = false;

		{
			std::lock_guard<std::mutex> lock(m_mutex);
			auto it = m_tasks.begin();
			// 1 遍历任务队列
			while(it!=m_tasks.end())
			{
				if(it->thread!=-1&&it->thread!=thread_id)
				{
					it++;
					tickle_me = true;
					continue;
				}

				// 2 取出任务
				assert(it->fiber||it->cb);
				task = *it;
				m_tasks.erase(it); 
				m_activeThreadCount++;
				break;
			}	
			tickle_me = tickle_me || (it != m_tasks.end());
		}

		if(tickle_me)
		{
			tickle();
		}

		// 3 执行任务
		if(task.fiber)
		{
			{
				std::lock_guard<std::mutex> lock(task.fiber->m_mutex);
				if(task.fiber->getState()!=Fiber::TERM)
				{
					task.fiber->resume();	
				}
			}
			m_activeThreadCount--;
			task.reset();
		}
		else if(task.cb)
		{
			std::shared_ptr<Fiber> cb_fiber = std::make_shared<Fiber>(task.cb);
			{
				std::lock_guard<std::mutex> lock(cb_fiber->m_mutex);
				cb_fiber->resume();			
			}
			m_activeThreadCount--;
			task.reset();	
		}
		// 4 无任务 -> 执行空闲协程
		else
		{
			// 系统关闭 -> idle协程将从死循环跳出并结束 -> 此时的idle协程状态为TERM -> 再次进入将跳出循环并退出run()
            if (idle_fiber->getState() == Fiber::TERM) 
            {
                break;
            }
			m_idleThreadCount++;
			idle_fiber->resume();
			m_idleThreadCount--;
		}
	}
}

void Scheduler::stop()
{
	if(stopping())
	{
		return;
	}

	m_stopping = true;	

    if (m_useCaller) 
    {
        assert(GetThis() == this);
    } 
    else 
    {
        assert(GetThis() != this);
    }
	
	for (size_t i = 0; i < m_threadCount; i++) 
	{
		tickle();
	}

	if (m_schedulerFiber) 
	{
		tickle();
	}

	if(m_schedulerFiber)
	{
		m_schedulerFiber->resume();
	}

	std::vector<std::shared_ptr<Thread>> thrs;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		thrs.swap(m_threads);
	}

	for(auto &i : thrs)
	{
		i->join();
	}
    DAG_LOG_DEBUG(g_logger) << this << " stopped";
}

void Scheduler::tickle()
{

}

void Scheduler::idle()
{
    DAG_LOG_INFO(g_logger) << "idle";
	while(!stopping())
	{
		Fiber::GetThis()->yield();
	}
}

bool Scheduler::stopping() 
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_stopping && m_tasks.empty() && m_activeThreadCount == 0;
}


}

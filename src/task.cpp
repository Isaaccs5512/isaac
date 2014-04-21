#include <functional>
#include <future>
#include <condition_variable>
//using  boost::posix_time::seconds;
#include "memory.hpp"
#include "error_exception.hpp"
#include "task.hpp"
#include "glog/logging.h"
#include <list>
#include <set>
#include <queue>
#include "debug.h"





class task : public std::tr1::enable_shared_from_this<task>
{
public:
	task():exit_(false){
		LOG(INFO)<<"Task Create...";
		}
	task( const task & ) = delete;
	~task(){  LOG(INFO)<<"task delete..\n";}
	task & operator =( const task &) = delete;

	void start();
	void stop()
	{
		exit_ = true;
		sync_.notify_one();
	}
	void set_job( const std::function<void()> & job)
	{
		{
			std::unique_lock<std::mutex> lock(mutex_);
			job_ = job;
		}
		sync_.notify_one();
	}
private:
	
	bool exit_;
	std::mutex mutex_;
	std::condition_variable sync_;
	std::function<void()> job_;
	std::thread::id       id_;

		
};



class task_pool 
{
public:
	task_pool(unsigned int pool_size = 128):max_size_(pool_size),stop_all_(true)
	{
		
	}
	~task_pool()
	{
	}
	void job_completed( const std::tr1::shared_ptr<task> & t)
	{
		
		std::lock_guard<std::mutex> lock(mutex_);
		bool need_to_notify = idle_tasks_.empty() && (!wait_for_running_jobs_.empty());
		busying_tasks_.erase(t);
		idle_tasks_.push_back(t);
		LOG(INFO)<<"after job_completed, current task size"<< idle_tasks_.size()<<std::endl;
		if( need_to_notify )
		{
			sync_.notify_one();
		}
	};
	
	void submit_job( const std::function<void()> & job)
	{
		if( stop_all_ )
		{
			LOG(FATAL)<<"task pool has stop....";
		}
		std::lock_guard<std::mutex> lock(mutex_);
		bool need_notify = wait_for_running_jobs_.empty();
		wait_for_running_jobs_.push(job);

		if( need_notify )
		{
			sync_.notify_one();
		}
	}
	void execute_job()
	{
	
		
		while(true)
		{
			std::unique_lock<std::mutex> lock(mutex_);
			while(!stop_all_ && wait_for_running_jobs_.empty() )
			{
				LOG(INFO)<<"wait for jobs to execute";
				sync_.wait(lock);
			}

			if( stop_all_ )
			{
				return;
			}
			while(!stop_all_ && idle_tasks_.empty())
			{
				LOG(INFO)<<"wait for idle taks to execute job";
				sync_.wait(lock);
			}
			if( stop_all_ )
			{
				return;
			}

			auto t = get_task();
			auto job = wait_for_running_jobs_.front();
			wait_for_running_jobs_.pop();

			t->set_job(job);
		}
	}
	void stop_all()
	{
		
		std::lock_guard<std::mutex> lock(mutex_); 
		stop_all_ = true;
		for( auto it : idle_tasks_ )
		{
			auto t = it;
			t->stop();
		}
		idle_tasks_.clear();
		for( auto it : busying_tasks_ )
		{
			auto t = it;
			t->stop();
		}
		while(!wait_for_running_jobs_.empty()){
			wait_for_running_jobs_.pop();
		}
		
		sync_.notify_one();
	}

	void start()
	{
		try
		{
			std::thread t( [this]{ execute_job();});
			t.detach();

			stop_all_ = false;
			allocate_tasks();
			
		}catch( std::exception & e )
		{
			LOG(FATAL) << "start tasks pool ... error"<<e.what();
		}
	}
protected:
	std::tr1::shared_ptr<task> get_task()
	{
		//std::lock_guard<std::mutex> lock(mutex_);
		if( ! idle_tasks_.empty() )
		{
			auto t = *idle_tasks_.begin();
			idle_tasks_.pop_front();
			busying_tasks_.insert(t);

			return t;
		}

		return std::tr1::shared_ptr<task>();

	}

	void allocate_tasks()
	{
		for( int i = 0 ; i < max_size_; i ++ )
		{
			std::tr1::shared_ptr<task> t( new task());
			try{
				t->start();
				idle_tasks_.push_back(t);
			}catch( std::exception & e)
			{
				LOG(INFO)<<"create task  error:"<<e.what()<<std::endl;
				break;
			}
		}
	}
private :
	unsigned int                              max_size_;
	std::list  < std::tr1::shared_ptr<task> > idle_tasks_;
	std::set  <  std::tr1::shared_ptr<task> > busying_tasks_;
	std::queue<  std::function<void()>      > wait_for_running_jobs_;
	std::mutex             				    mutex_;
	std::condition_variable                 sync_;
	bool stop_all_;
};

static task_pool *  tasks = nullptr;
static std::once_flag init_flag;
static std::once_flag finit_flag;

void task::start()
{
	auto job_proxy = [this] (){
		
		id_ = std::this_thread::get_id();
		
		while( !exit_ )
		{
			
			std::unique_lock<std::mutex> lock(mutex_);
			
			
			if( job_ )
			{
				try
				{
					job_();
				}catch( std::exception & e)
				{
					LOG(ERROR)<<"execute job occur exception:"<<e.what();
				}catch(...)
				{
					LOG(ERROR)<<"execute job occur semaget flat";
				}
				job_ = std::function<void()>();
				tasks->job_completed( shared_from_this() );
			}else{
				
				sync_.wait(lock);
				
			}
		}
	};
	
	std::thread t(job_proxy);
	
	t.detach();
}


void run_job(const std::function<void()> & job )
{	
	if( tasks != nullptr)
		tasks->submit_job(job);
	
}
void task_pool_init( unsigned max_task_size)
{
	std::call_once(init_flag,[max_task_size]{ 
		tasks = new task_pool(max_task_size);
		tasks->start();
	});
}
void task_pool_finit()
{
	LOG(INFO)<<"task_pool_finit()";
   std::call_once(finit_flag,[]{ tasks->stop_all();});
}



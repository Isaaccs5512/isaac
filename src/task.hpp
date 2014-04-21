#ifndef _APP_TAKS_HPP_
#define _APP_TAKS_HPP_
#include <functional>
  
/*
	异步执行的任务，从线程池中分配一个空闲的线程来执行该任务.
	如果没有空闲的线程可以执行，则进入队列排队
*/
void task_pool_init(unsigned max_task_size = 128);
void run_job(const std::function<void()> & job );
void task_pool_finit();

#endif

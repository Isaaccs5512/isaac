#ifndef _APP_TAKS_HPP_
#define _APP_TAKS_HPP_
#include <functional>
  
/*
	�첽ִ�е����񣬴��̳߳��з���һ�����е��߳���ִ�и�����.
	���û�п��е��߳̿���ִ�У����������Ŷ�
*/
void task_pool_init(unsigned max_task_size = 128);
void run_job(const std::function<void()> & job );
void task_pool_finit();

#endif

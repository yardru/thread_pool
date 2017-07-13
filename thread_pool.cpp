#include "thread_pool.h"


ds::THREAD_POOL::THREAD_POOL(COUNT threadsNumber)
{
   for (COUNT i = 0; i < threadsNumber; i++) {
      THREAD * pThread = new THREAD(*this);
      threads.push_back(std::unique_ptr<THREAD>(pThread));
      freeThreads.push(pThread);
   }
}


std::future<int> ds::THREAD_POOL::AddTask(const TASK & task)
{
   PACKAGED_TASK packagedTask(task);
   auto future = packagedTask.get_future();

   std::unique_lock<std::mutex> lock(mutex);
   if (!freeThreads.empty()) {
      freeThreads.front()->SetTask(std::move(packagedTask));
      freeThreads.pop();
   } else {
      packagedTasks.push(std::move(packagedTask));
   }

   return future;
}


void ds::THREAD_POOL::AddFreeThread(THREAD & thread)
{
   std::unique_lock<std::mutex> lock(mutex);
   if (!packagedTasks.empty()) {
      thread.SetTask(std::move(packagedTasks.front()));
      packagedTasks.pop();
   }
   else {
      freeThreads.push(&thread);
   }
}


ds::THREAD_POOL::THREAD::THREAD(THREAD_POOL & pool) : pool(pool),
   stdThread(ThreadFunc, std::ref(*this)), isClose(false)
{
}


ds::THREAD_POOL::THREAD::~THREAD(void)
{
   isClose = true;
   isTaskUpdated.notify_all();
   if (stdThread.joinable()) {
      stdThread.join();
   }
}


void ds::THREAD_POOL::THREAD::SetTask(PACKAGED_TASK && packagedTask)
{
   std::unique_lock<std::mutex> lock(mutex);
   task = std::move(packagedTask);
   isTaskUpdated.notify_all();
}


void ds::THREAD_POOL::THREAD::ThreadFunc(THREAD & thread)
{
   std::mutex utilMutex;
   while (!thread.isClose) {
      std::unique_lock<std::mutex> lock(utilMutex);
      thread.isTaskUpdated.wait(lock, [&task = thread.task, &isClose = thread.isClose]() {
         return task.valid() || isClose;
      });
      if (thread.task.valid()) {
         thread.task();
         thread.SetTask(PACKAGED_TASK());
         thread.pool.AddFreeThread(thread);
      }
   }
}

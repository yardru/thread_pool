#include <assert.h>
#include "thread_pool.h"


ds::THREAD_POOL::THREAD_POOL(COUNT threadsNumber) : isClose(false)
{
   assert(threadsNumber > 0);
   for (COUNT i = 0; i < threadsNumber; i++) {
      threads.emplace_back(*this);
   }
}


ds::THREAD_POOL::~THREAD_POOL()
{
   std::unique_lock<std::mutex> lock(cvMutex);
   isClose = true;
   isUpdated.notify_all();
}


bool ds::THREAD_POOL::IsClose() const
{
   return isClose;
}


ds::THREAD_POOL::PTASK ds::THREAD_POOL::GetTask()
{
   std::unique_lock<std::mutex> lock(queueMutex);
   PTASK task;
   if (!tasks.empty()) {
      task = std::move(tasks.front());
      tasks.pop();
   }
   return task;
}


std::mutex & ds::THREAD_POOL::GetMutex()
{
   return cvMutex;
}


ds::THREAD_POOL::THREAD::THREAD(THREAD_POOL & pool) : pool(pool),
   end(std::async(std::launch::async, [this]() { Run(); }))
{
}


ds::THREAD_POOL::THREAD::~THREAD()
{
   end.get();
}


void ds::THREAD_POOL::THREAD::Run()
{
   PTASK task;
   while (!pool.IsClose()) {
      if (!(task = pool.GetTask())) {
         std::unique_lock<std::mutex> lock(pool.GetMutex());
         if (pool.IsClose()) {
            break;
         }
         if (!(task = pool.GetTask())) {
            pool.isUpdated.wait(lock);
            continue;
         }
      }
      task->Call();
   }
}

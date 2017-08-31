#include <assert.h>
#include "thread_pool.h"


ds::THREAD_POOL::THREAD_POOL(COUNT threadsNumber) : isClose(false)
{
   assert(threadsNumber > 0);
   for (COUNT i = 0; i < threadsNumber; i++) {
      threads.emplace_back([this]() { ThreadLoop(); });
   }
}


ds::THREAD_POOL::~THREAD_POOL()
{
   std::unique_lock<std::mutex> lock(mutex);
   isClose = true;
   isUpdated.notify_all();
   lock.unlock();
   for (auto & thread : threads) {
      thread.join();
   }
}


void ds::THREAD_POOL::ThreadLoop()
{
   while (true) {
      std::unique_lock<std::mutex> lock(mutex);
      if (isClose) {
         break;
      }
      if (!tasks.empty()) {
         TASK task = std::move(tasks.front());
         tasks.pop();
         lock.unlock();
         task();
      } else {
         isUpdated.wait(lock);
      }
   }
}

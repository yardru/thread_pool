#ifndef _DS_THREAD_POOL_H_
#define _DS_THREAD_POOL_H_

#include <list>
#include <queue>
#include <thread>
#include <future>
#include <functional>
#include <mutex>

namespace ds {

class THREAD_POOL {
public:
   using COUNT = unsigned int;

   explicit THREAD_POOL(COUNT threadsNumber);
   ~THREAD_POOL();

   // Return std::future<RET> object, where RET - func return type
   template <typename FUNC>
   auto AddTask(FUNC && func);

private:
   using TASK = std::function<void(void)>;

   void ThreadLoop(void);

   bool isClose;
   std::mutex mutex;
   std::condition_variable isUpdated;
   std::queue<TASK> tasks;
   std::vector<std::thread> threads;
};

} // namespace ds

#include "thread_pool.hpp"

#endif // _DS_THREAD_POOL_H_

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
   using TASK = std::function<int(void)>;
   using PACKAGED_TASK = std::packaged_task<int(void)>;

   THREAD_POOL(COUNT threadsNumber);
   std::future<int> AddTask(const TASK & task);
private:
   class THREAD {
   public:
      THREAD(THREAD_POOL & pool);
      ~THREAD(void);
      void SetTask(PACKAGED_TASK && packagedTask);
   private:
      static void ThreadFunc(THREAD & thread);

      THREAD_POOL & pool;
      PACKAGED_TASK task;
      std::mutex mutex;
      std::condition_variable isTaskUpdated;
      bool isClose;
      std::thread stdThread;
   };

   void AddFreeThread(THREAD & thread);

   std::queue<PACKAGED_TASK> packagedTasks;
   std::vector<std::unique_ptr<THREAD>> threads;
   std::queue<THREAD *> freeThreads;
   std::mutex mutex;
};

} // namespace ds

#endif // _DS_THREAD_POOL_H_

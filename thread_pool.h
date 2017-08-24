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
   template <typename FUNC, typename... ARGS>
   auto AddTask(FUNC && func, ARGS &&... args);

private:
   class TASK {
   public:
      virtual void Call() = 0;
      virtual ~TASK() = default;
   };
   using PTASK = std::unique_ptr<TASK>;

   class THREAD {
   public:
      explicit THREAD(THREAD_POOL & pool);
      ~THREAD();
   private:
      void Run(void);

      THREAD_POOL & pool;
      std::future<void> end;
   };

   bool IsClose() const;
   PTASK GetTask();
   std::mutex & GetMutex();

   template <typename FUNC, typename... ARGS, std::size_t... I>
   static constexpr decltype(auto) ApplyFunc(FUNC && func, std::tuple<ARGS...> & args, std::index_sequence<I...>);

   bool isClose;
   std::mutex cvMutex;
   std::mutex queueMutex;
   std::condition_variable isUpdated;
   std::queue<PTASK> tasks;
   std::list<THREAD> threads;
};

} // namespace ds

#include "thread_pool.hpp"

#endif // _DS_THREAD_POOL_H_

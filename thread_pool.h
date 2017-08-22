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

   // Only because of errors with std::function<RET &(void)>
   template <typename RET, typename FUNC, typename... ARGS>
   class REAL_TASK_BASE;
   template <typename RET, typename FUNC, typename... ARGS>
   class REAL_TASK;

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

   // See std::apply from c++17
   // Implementation here because of error C3779
   template <typename FUNC, typename... ARGS, std::size_t... I>
   static constexpr decltype(auto) ApplyFunc(FUNC && func, std::tuple<ARGS...> & args, std::index_sequence<I...>)
   {
      //return std::forward<FUNC>(func)(std::get<I>(std::forward<TUPLE>(args))...);
      return std::forward<FUNC>(func)(std::forward<ARGS>(std::get<I>(args))...);
   }

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

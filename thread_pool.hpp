#ifdef _DS_THREAD_POOL_H_
#ifndef _DS_THREAD_POOL_HPP_
#define _DS_THREAD_POOL_HPP_

template <typename FUNC>
auto ds::THREAD_POOL::AddTask(FUNC && func)
{
   using RET = decltype(std::forward<FUNC>(func)());
   using PACKAGED_TASK = std::packaged_task<RET(void)>;

   PACKAGED_TASK packaged_task(std::forward<FUNC>(func));
   std::future<RET> future = packaged_task.get_future();

   // because std::function doesn't work with noncopyable functors
   class COPYABLE_PACKAGED_TASK {
   public:
      COPYABLE_PACKAGED_TASK(PACKAGED_TASK && packaged_task) : packaged_task(std::move(packaged_task)) {}
      COPYABLE_PACKAGED_TASK(const COPYABLE_PACKAGED_TASK & copyable_packaged_task) : packaged_task(std::move(copyable_packaged_task.packaged_task)) {}
      void operator()(void) { packaged_task(); }
   private:
      mutable PACKAGED_TASK packaged_task;
   } copyable_packaged_task(std::move(packaged_task));

   std::unique_lock<std::mutex> lock(mutex);
   tasks.push([packaged_task = copyable_packaged_task]() mutable { packaged_task(); });
   isUpdated.notify_one();
   lock.unlock();

   return future;
}


#endif // _DS_THREAD_POOL_HPP_
#endif // _DS_THREAD_POOL_H_

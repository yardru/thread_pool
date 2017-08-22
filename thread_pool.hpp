#ifdef _DS_THREAD_POOL_H_
#ifndef _DS_THREAD_POOL_HPP_
#define _DS_THREAD_POOL_HPP_


template <typename RET, typename FUNC, typename... ARGS>
class ds::THREAD_POOL::REAL_TASK_BASE : public ds::THREAD_POOL::TASK {
public:
   REAL_TASK_BASE(FUNC && func_, ARGS &&... args_) : func(std::forward<FUNC>(func_)), args(std::forward<ARGS>(args_)...) {}
   std::future<RET> GetFuture() { return promise.get_future(); }
protected:
   FUNC func;
   std::tuple<ARGS...> args;
   std::promise<RET> promise;
};


template <typename RET, typename FUNC, typename... ARGS>
class ds::THREAD_POOL::REAL_TASK : public ds::THREAD_POOL::REAL_TASK_BASE<RET, FUNC, ARGS...> {
public:
   REAL_TASK(FUNC && func_, ARGS &&... args_) : REAL_TASK_BASE(std::forward<FUNC>(func_), std::forward<ARGS>(args_)...) {}
   virtual void Call() override
   {
      promise.set_value(ApplyFunc(std::forward<FUNC>(func), args, std::index_sequence_for<ARGS...>()));
   }
};


template <typename FUNC, typename... ARGS>
class ds::THREAD_POOL::REAL_TASK<void, FUNC, ARGS...> : public ds::THREAD_POOL::REAL_TASK_BASE<void, FUNC, ARGS...> {
public:
   REAL_TASK(FUNC && func_, ARGS &&... args_) : REAL_TASK_BASE(std::forward<FUNC>(func_), std::forward<ARGS>(args_)...) {}
   virtual void Call() override
   {
      ApplyFunc(std::forward<FUNC>(func), args, std::index_sequence_for<ARGS...>());
      promise.set_value();
   }
};


template <typename FUNC, typename... ARGS>
auto ds::THREAD_POOL::AddTask(FUNC && func, ARGS &&... args)
{
   using RET = decltype(std::forward<FUNC>(func)(std::forward<ARGS>(args)...));
   auto task = new REAL_TASK<RET, FUNC, ARGS...>(std::forward<FUNC>(func), std::forward<ARGS>(args)...);

   std::unique_lock<std::mutex> queueLock(queueMutex, std::defer_lock);
   std::unique_lock<std::mutex> cvLock(cvMutex, std::defer_lock);
   std::lock(queueLock, cvLock);

   tasks.push(PTASK(task));
   isUpdated.notify_one();

   queueLock.unlock();
   cvLock.unlock();

   return task->GetFuture();
}

#endif // _DS_THREAD_POOL_HPP_
#endif // _DS_THREAD_POOL_H_

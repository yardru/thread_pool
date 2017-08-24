#ifdef _DS_THREAD_POOL_H_
#ifndef _DS_THREAD_POOL_HPP_
#define _DS_THREAD_POOL_HPP_


// See std::apply from c++17
template <typename FUNC, typename... ARGS, std::size_t... I>
constexpr decltype(auto) ds::THREAD_POOL::ApplyFunc(FUNC && func, std::tuple<ARGS...> & args, std::index_sequence<I...>)
{
   return std::forward<FUNC>(func)(std::forward<ARGS>(std::get<I>(args))...);
}


template <typename FUNC, typename... ARGS>
auto ds::THREAD_POOL::AddTask(FUNC && func, ARGS &&... args)
{
   using RET = decltype(std::forward<FUNC>(func)(std::forward<ARGS>(args)...));

   class REAL_TASK : public TASK {
   public:
      REAL_TASK(FUNC && func, ARGS &&... args) : func(std::forward<FUNC>(func)), args(std::forward<ARGS>(args)...),
         packaged_task([](FUNC && func, std::tuple<ARGS...> & args) -> RET {
            return ApplyFunc(std::forward<FUNC>(func), args, std::index_sequence_for<ARGS...>());
         }) {}
      std::future<RET> GetFuture() { return packaged_task.get_future(); }
      virtual void Call() override { packaged_task(std::forward<FUNC>(func), args); }
   private:
      FUNC func;
      std::tuple<ARGS...> args;
      std::packaged_task<RET(FUNC &&, std::tuple<ARGS...> &)> packaged_task;
   } * task = new REAL_TASK(std::forward<FUNC>(func), std::forward<ARGS>(args)...);

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

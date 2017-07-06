#ifndef _DS_THREAD_POOL_H_
#define _DS_THREAD_POOL_H_

#include <future>
#include <functional>

namespace ds {

class THREAD_POOL {
public:
   using COUNT = unsigned int;
   using TASK = std::function<int(void)>;

   THREAD_POOL(COUNT threads_number);
   std::future<int> AddTask(const TASK & task);
};

} // namespace ds

#endif // _DS_THREAD_POOL_H_

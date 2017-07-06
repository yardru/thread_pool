#include <chrono>
#include <vector>

#include "thread_pool.h"


using namespace std;
using namespace std::chrono;

using dsTHREAD_POOL = ds::THREAD_POOL;


int main(void)
{
   dsTHREAD_POOL pool(10); // num_threads

   auto start = high_resolution_clock::now();

   vector<future<int>> result_list;
   for (int k = 0; k < 100; ++k) {
      result_list.push_back(pool.AddTask([k] {
         this_thread::sleep_for(100ms); // simulate work
         return k * k;
      }));
   }

   int sum = 0;
   for (auto & p : result_list) {
      sum += p.get();
   }

   auto end = high_resolution_clock::now();
   duration<double> elapsed = end - start;

   printf("result = %d\n", sum);
   printf("finished in %.2f sec\n", elapsed.count());

   return 0;
}

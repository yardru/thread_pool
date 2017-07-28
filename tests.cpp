#include <chrono>
#include <vector>

#include <gtest\gtest.h>

#include "thread_pool.h"


using namespace std;
using namespace std::chrono;

using dsTHREAD_POOL = ds::THREAD_POOL;

static int n = 0;

TEST(General, MainTest)
{
   n++;
   dsTHREAD_POOL pool(n); // num_threads

   auto start = high_resolution_clock::now();

   const int m = 100;
   vector<future<int>> result_list;
   for (int k = 0; k < m; ++k) {
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

   double expected = 0.1 * m / n;
   ASSERT_NEAR(elapsed.count(), expected, 0.5);
   ASSERT_EQ(sum, (m - 1) * m * (2 * m - 1) / 6);
}

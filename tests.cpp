#include <iostream>
#include <chrono>
#include <vector>

#include <gtest\gtest.h>

#include "thread_pool.h"


using namespace std;
using namespace std::chrono;

using dsTHREAD_POOL = ds::THREAD_POOL;


TEST(GeneralTest, Base)
{
   int n = 10; // num_threads
   dsTHREAD_POOL pool(n);

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
   ASSERT_NEAR(elapsed.count(), expected, 0.1);
   ASSERT_EQ(sum, (m - 1) * m * (2 * m - 1) / 6);
}


TEST(GeneralTest, DoubleMain)
{
   dsTHREAD_POOL pool(10);

   auto start = high_resolution_clock::now();

   vector<future<int>> result_list;
   for (int k = 0; k < 100; ++k) {
      result_list.push_back(pool.AddTask([k] {
         this_thread::sleep_for(100ms); // simulate work
         return k * k;
      }));
   }

   int sum = 0;
   for (int i = 0; i < 50; i++) {
      sum += result_list[i].get();
   }

   for (int k = 0; k < 100; ++k) {
      result_list.push_back(pool.AddTask([k] {
         this_thread::sleep_for(100ms); // simulate work
         return k * k;
      }));
   }

   this_thread::sleep_for(1s); // some useful work...

   for (int i = 150; i < 200; i++) {
      sum += result_list[i].get();
   }

   auto end = high_resolution_clock::now();
   duration<double> elapsed = end - start;

   double expected = 2;
   ASSERT_LE(elapsed.count(), expected * 1.1);
}


TEST(SlowFuncTest, NotAsked)
{
   dsTHREAD_POOL pool(1);
   pool.AddTask([]() { this_thread::sleep_for(1s); });
}


TEST(SlowFuncTest, Asked)
{
   dsTHREAD_POOL pool(1);
   pool.AddTask([]() { this_thread::sleep_for(1s); }).get();
}

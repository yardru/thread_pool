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


TEST(GeneralTest, DiffFuncs)
{
   dsTHREAD_POOL pool(10);
   int k = 1;
   auto f1 = pool.AddTask([]() { return 1; });
   auto f2 = pool.AddTask([](const auto & a, const auto & b) { return a == b; }, 2.5, 2.4);
   auto f3 = pool.AddTask([&k](int a) { k *= a; }, 7);
   int r1 = f1.get();
   bool r2 = f2.get();
   f3.get();
   ASSERT_EQ(r1, 1);
   ASSERT_EQ(r2, false);
   ASSERT_EQ(k, 7);
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


TEST(RefArgsTest, RValueArgs)
{
   dsTHREAD_POOL pool(1);
   std::future<void> f;
   std::vector<int> v;

   {
      class FUNC {
      public:
         FUNC(std::vector<int> & v) : v(v) {}
         void operator()(std::vector<int> && v)
         {
            this->v = std::move(v);
         }
      private:
         std::vector<int> & v;
      } func(v);
      f = pool.AddTask(func, std::vector<int>{1, 2, 3});
   }

   f.get();
   ASSERT_EQ(v[0] + v[1] + v[2], 6);
}


struct NONCOPYABLE {
public:
   explicit NONCOPYABLE(int val) : val(val) {}
   NONCOPYABLE(const NONCOPYABLE & rhs) = delete;
   NONCOPYABLE(NONCOPYABLE && rhs) = delete;
   NONCOPYABLE & operator=(const NONCOPYABLE & rhs) = delete;
   NONCOPYABLE & operator=(NONCOPYABLE && rhs) = delete;
   int & GetIncVal() { return ++val; }
private:
   int val;
};


TEST(RefArgsTest, LValueArgsRValueRet)
{
   NONCOPYABLE ncp(0);

   {
      dsTHREAD_POOL pool(1);
      class FUNC {
      public:
         int operator()(NONCOPYABLE & ncp) { return ncp.GetIncVal(); }
      } func;
      auto f = pool.AddTask(func, ncp);
      decltype(auto) b = f.get();
      b++;
   }
   ASSERT_EQ(ncp.GetIncVal(), 2);
}


TEST(RefArgsTest, LValueArgsLValueRet)
{
   NONCOPYABLE ncp(0);

   {
      dsTHREAD_POOL pool(1);
      class FUNC {
      public:
         NONCOPYABLE & operator()(NONCOPYABLE & ncp) { return ncp; }
      } func;
      auto f = pool.AddTask(func, ncp);
      decltype(auto) b = f.get().GetIncVal();
      b++;
   }

   ASSERT_EQ(ncp.GetIncVal(), 3);
}

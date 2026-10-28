#include "concurrency_tests.h"

#include "../jtk/concurrency.h"
#include <thread>
#include <stdint.h>

#include "test_assert.h"

#include <iostream>

namespace jtk
  {

  spinlock_rw g_lock;
  uint64_t g_counter = 0;
  const uint32_t count = 5000;

  void add_job()
    {
    for (int i = 0; i < count; ++i)
      {
      g_lock.lock();
      ++g_counter;
      std::cout << "Thread " << get_thread_id() << std::endl;
      g_lock.unlock();
      }
    }

  void read_job()
    {
    for (int i = 0; i < count; ++i)
      {
      g_lock.lock_read();
      std::cout << g_counter << std::endl;
      g_lock.unlock();
      }
    }

  void spin_lock_rw_test()
    {
    std::thread th1(add_job);
    std::thread th2(add_job);

    std::thread th3(read_job);
    std::thread th4(read_job);
    std::thread th5(read_job);

    th1.join();
    th2.join();
    th3.join();
    th4.join();
    th5.join();

    TEST_EQ(10000, g_counter);
    }

  // thread_pool::stop right after the work: every worker leaves its loop and stop returns. Repeated,
  // as a lost wakeup in stop (the notification between a worker's test of _quit and its wait) hung
  // the join only now and then.
  void thread_pool_stop_test()
    {
    for (int round = 0; round < 200; ++round)
      {
      thread_pool tp;
      tp.init();
      std::atomic<int> sum(0);
      pooled_parallel_for(0, 64, [&](int i) { sum += i; }, tp);
      TEST_EQ(64 * 63 / 2, sum.load());
      tp.stop();
      TEST_EQ((size_t)0, tp.size());
      }
    }

  }

void run_all_concurrency_tests()
  {
  using namespace jtk;
  spin_lock_rw_test();
  thread_pool_stop_test();
  }
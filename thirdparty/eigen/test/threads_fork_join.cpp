// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2025 Weiwei Kong <weiweikong@google.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#define EIGEN_USE_THREADS
#include "main.h"
#include "Eigen/ThreadPool"

struct TestData {
  std::unique_ptr<ThreadPool> tp;
  std::vector<double> data;
};

TestData make_test_data(int num_threads, int num_shards) {
  return {std::make_unique<ThreadPool>(num_threads), std::vector<double>(num_shards, 1.0)};
}

static void test_parallel_for(int granularity) {
  // Test correctness.
  const int kNumTasks = 100000;
  TestData test_data = make_test_data(/*num_threads=*/4, kNumTasks);
  std::atomic<uint64_t> sum(0);
  std::function<void(Index, Index)> binary_do_fn = [&](Index i, Index j) {
    for (int k = i; k < j; ++k)
      for (uint64_t new_sum = sum; !sum.compare_exchange_weak(new_sum, new_sum + test_data.data[k]);) {
      };
  };
  ForkJoinScheduler::ParallelFor(0, kNumTasks, granularity, std::move(binary_do_fn), test_data.tp.get());
  VERIFY_IS_EQUAL(sum.load(), kNumTasks);
}

static void test_async_parallel_for() {
  // Test correctness.
  // NOTE: Granularity and type of `do_func` are checked in the synchronous tests.
  const int kNumThreads = 4;
  const int kNumTasks = 100;
  const int kNumAsyncCalls = kNumThreads * 4;
  TestData test_data = make_test_data(kNumThreads, kNumTasks);
  std::atomic<uint64_t> sum(0);
  std::function<void(Index, Index)> binary_do_fn = [&](Index i, Index j) {
    for (Index k = i; k < j; ++k) {
      for (uint64_t new_sum = sum; !sum.compare_exchange_weak(new_sum, new_sum + test_data.data[i]);) {
      }
    }
  };
  Barrier barrier(kNumAsyncCalls);
  std::function<void()> done = [&]() { barrier.Notify(); };
  for (int k = 0; k < kNumAsyncCalls; ++k) {
    test_data.tp->Schedule([&]() {
      ForkJoinScheduler::ParallelForAsync(0, kNumTasks, /*granularity=*/1, binary_do_fn, done, test_data.tp.get());
    });
  }
  barrier.Wait();
  VERIFY_IS_EQUAL(sum.load(), kNumTasks * kNumAsyncCalls);
}

EIGEN_DECLARE_TEST(fork_join) {
  CALL_SUBTEST(test_parallel_for(1));
  CALL_SUBTEST(test_parallel_for(2));
  CALL_SUBTEST(test_async_parallel_for());
}

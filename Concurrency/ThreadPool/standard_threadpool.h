#pragma once

#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include <iostream>

#include "../ThreadSafeQueue/threadsafe_queue.h"
#include "threadpool.h"

namespace concurrency {
template <typename F, typename R = typename std::result_of<F()>::type>
class standard_threadpool : public threadpool<F> {
private:
  threadsafe_queue<std::packaged_task<void()>> queue;
  std::vector<std::thread> threads;

  void init_threads() {
    for (int i = 0; i < threadpool<F>::max_threads; i++) {
      threads.emplace_back([&]() {
        while (auto f = queue.pop()) {
          (*f)();
        }
      });
    }
  }

public:
  standard_threadpool(int max_threads) : threadpool<F>(max_threads) {
    init_threads();
  }

  void set_abort_flag() { queue.set_abort_flag(); }

  [[nodiscard]] bool is_aborted() { return queue.is_aborted(); }

  std::future<R> push(F f) {
    std::packaged_task<R()> temp_task(std::move(f));
    auto ret = temp_task.get_future();

    std::packaged_task<void()> task(std::move(temp_task));

    if (queue.push(std::move(task)))
      return ret;
    else
      return {};
  }

  ~standard_threadpool() {
    queue.wait_until_empty();
    queue.set_abort_flag();

    for (auto &t : threads) {
      t.join();
    }
  }
};

} // namespace concurrency

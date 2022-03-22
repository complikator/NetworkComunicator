#pragma once

#include <functional>
#include <future>
#include <type_traits>

namespace concurrency {

template <typename F, typename R = typename std::result_of<F()>::type>
class threadpool {
public:
  int max_threads;

public:
  threadpool(int max_threads) : max_threads(max_threads) {}

  virtual std::future<R> push(F) = 0;

  threadpool(const threadpool &) = delete;
  threadpool(const threadpool &&) = delete;
};
} // namespace concurrency

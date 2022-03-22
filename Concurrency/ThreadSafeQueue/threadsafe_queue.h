#include <condition_variable>
#include <deque>
#include <future>
#include <memory>
#include <mutex>
#include <utility>
#include <optional>

namespace concurrency {
template <class T> class threadsafe_queue {
public:
  [[nodiscard]] std::optional<T> pop() {
    auto l = lock();
    cv.wait(l, [&]() { return is_aborted() || !data.empty(); });

    if (is_aborted())
      return {};
    auto r = std::move(data.front());
    data.pop_front();
    cv.notify_all();
    return r;
  }

  bool push(T new_value) {
    auto l = lock();
    if (is_aborted())
      return false;
    data.push_back(std::move(new_value));
    cv.notify_one();
    return true;
  }

  void set_abort_flag() {
    auto l = lock();
    aborted_flag.store(true);
    data.clear();
    cv.notify_all();
  }

  [[nodiscard]] bool is_aborted() const { return aborted_flag; }

  void wait_until_empty() {
    auto l = lock();
    cv.wait(l, [&]() { return data.empty(); });
  }

private:
  std::mutex m;
  std::condition_variable cv;
  std::deque<T> data;
  std::atomic<bool> aborted_flag{false};

  std::unique_lock<std::mutex> lock() {
    return std::unique_lock<std::mutex>(m);
  }
};
} // namespace concurrency

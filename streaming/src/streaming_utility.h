#ifndef RAY_STREAMING_UTILITY_H
#define RAY_STREAMING_UTILITY_H
#include <string>

#include "streaming.h"

namespace ray {
namespace streaming {

typedef std::map<std::string, std::string> TagMap;

class StreamingUtility {
 public:
  static std::string Byte2hex(const uint8_t *data, uint32_t data_size);

  static std::string Hexqid2str(const std::string &q_id_hex);

  static std::string Qid2EdgeInfo(const ray::ObjectID &q_id);

  static std::string GetHostname();

  static void Split(const ray::ObjectID &q_id, std::vector<std::string> &q_splited_vec);

  template <typename T>
  static std::string join(const T &v, const std::string &delimiter,
                          const std::string &prefix = "",
                          const std::string &suffix = "") {
    std::stringstream ss;
    size_t i = 0;
    ss << prefix;
    for (const auto &elem : v) {
      if (i != 0) {
        ss << delimiter;
      }
      ss << elem;
      i++;
    }
    ss << suffix;
    return ss.str();
  }

  template <class InputIterator>
  static std::string join(InputIterator first, InputIterator last,
                          const std::string &delim, const std::string &arround = "") {
    std::string a = arround;
    while (first != last) {
      a += std::to_string(*first);
      first++;
      if (first != last) a += delim;
    }
    a += arround;
    return a;
  }

  template <class InputIterator>
  static std::string join(InputIterator first, InputIterator last,
                          std::function<std::string(InputIterator)> func,
                          const std::string &delim, const std::string &arround = "") {
    std::string a = arround;
    while (first != last) {
      a += func(first);
      first++;
      if (first != last) a += delim;
    }
    a += arround;
    return a;
  }
};

class AutoSpinLock {
 public:
  explicit AutoSpinLock(std::atomic_flag &lock) : lock_(lock) {
    while (lock_.test_and_set(std::memory_order_acquire))
      ;
  }
  ~AutoSpinLock() { unlock(); }
  void unlock() { lock_.clear(std::memory_order_release); }

 private:
  std::atomic_flag &lock_;
};

inline int64_t current_sys_time_ms() {
  std::chrono::milliseconds ms_since_epoch =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch());
  return ms_since_epoch.count();
}

}  // namespace streaming
}  // namespace ray

#endif  // RAY_STREAMING_UTILITY_H

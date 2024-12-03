// Copyright 2021 Google LLC
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef LOG_H
#define LOG_H

#include <cerrno>
#include <chrono>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>

#include <syslog.h>

template <typename... Args>
std::string StrCat(Args&&... args) {
  std::ostringstream out;
  (out << ... << std::forward<Args>(args));
  return std::move(out).str();
}

enum class LogLevel {
  DEBUG = LOG_DEBUG,
  INFO = LOG_INFO,
  WARNING = LOG_WARNING,
  ERROR = LOG_ERR,
};

extern LogLevel g_log_level;

void SetLogLevel(LogLevel level);

#define LOG_IS_ON(level) (LogLevel::level <= g_log_level)

// Accumulates a log message and logs it.
class Logger {
 public:
  ~Logger();

  explicit Logger(LogLevel const level, error_t err = -1)
      : level_(level), err_(err) {}

  Logger(const Logger&) = delete;

  Logger&& operator<<(const auto& a) && {
    oss_ << a;
    return std::move(*this);
  }

 private:
  const LogLevel level_;
  const error_t err_;
  std::ostringstream oss_;
};

#define LOG(level)                    \
  if (LogLevel::level <= g_log_level) \
  Logger(LogLevel::level)

#define PLOG(level)                   \
  if (LogLevel::level <= g_log_level) \
  Logger(LogLevel::level, errno)

// Timer for debug logs.
struct Timer {
  using Clock = std::chrono::steady_clock;

  // Start time.
  Clock::time_point start = Clock::now();

  // Resets this timer.
  void Reset() { start = Clock::now(); }

  // Elapsed time in milliseconds.
  auto Milliseconds() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() -
                                                                 start)
        .count();
  }

  friend std::ostream& operator<<(std::ostream& out, const Timer& timer) {
    return out << timer.Milliseconds() << " ms";
  }
};

// Generates a regular beat for logging of lengthy operations.
class Beat {
 private:
  using Clock = std::chrono::steady_clock;
  using TimePoint = Clock::time_point;
  using Duration = Clock::duration;

  // Beat period.
  const Duration period_ = std::chrono::milliseconds(1000);

  // Next beat time.
  TimePoint next_ = Clock::now() + period_;

  // Number of produced beats.
  int count_ = 0;

 public:
  // Is it time for the next beat?
  explicit operator bool() {
    const TimePoint now = Clock::now();
    if (now < next_)
      return false;

    count_ += 1;
    next_ = now + period_;
    return true;
  }

  // Gets the number of produced beats.
  int Count() const { return count_; }
};

#endif  // LOG_H

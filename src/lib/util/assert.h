#pragma once

#include "src/lib/util/logger.h"

#include <cassert>

namespace NotAGame {

inline void Fail() noexcept { assert(false); }

inline void Unreachable() noexcept { Fail(); }

template <void (*Fn)()> class LogAndAction {
public:
  LogAndAction() noexcept : Logger_{GetLogger()} {}
  ~LogAndAction() noexcept { Fn(); }

  template <typename T> LogAndAction &operator<<(const T &Value) noexcept {
    Logger_ << Value;
    return *this;
  }

private:
  Logger Logger_;
};

inline auto LogFatal() noexcept { return LogAndAction<Fail>(); }
inline auto LogUnreachable() noexcept { return LogAndAction<Unreachable>(); }

} // namespace NotAGame

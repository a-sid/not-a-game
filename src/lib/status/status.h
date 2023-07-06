#pragma once

#include <sstream>

namespace NotAGame {

enum class ErrorCode {
  Success = 0,
  MapError = 1,
  MaxCode = MapError,
};

class Status {
public:
  explicit Status(ErrorCode Code = ErrorCode::Success, const std::string &Message = {})
      : Code_{Code}, Message_{Message} {}
  explicit Status(ErrorCode Code, std::string_view Message) : Code_{Code} { Message_ << Message; }

  ErrorCode Code() { return Code_; }
  std::string Message() { return Message_.str(); }

  template <typename T> Status &operator<<(const T &Part) {
    Message_ << Part;
    return *this;
  }

  static Status Error(ErrorCode Code, const std::string &Message = {}) {
    return Status{Code, Message};
  }

  static Status Success() { return Status{}; }

private:
  ErrorCode Code_;
  std::stringstream Message_;
};

} // namespace NotAGame

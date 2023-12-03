#pragma once

#include <sstream>
#include <variant>

namespace NotAGame {

enum class ErrorCode {
  Success = 0,
  LobbyFull,
  PlayerNotFound,
  ObjectNotFound,
  GameSlotBusy,
  WrongState,
  WrongPlayer,
  MapError,
  MaxCode = MapError,
};

class Status {
public:
  explicit Status(ErrorCode Code = ErrorCode::Success, const std::string &Message = {}) noexcept
      : Code_{Code}, Message_{Message} {}
  explicit Status(ErrorCode Code, std::string_view Message) : Code_{Code} { Message_ << Message; }
  ErrorCode Code() const noexcept { return Code_; }
  std::string Message() const noexcept { return Message_.str(); }

  bool IsSuccess() const noexcept { return Code_ == ErrorCode::Success; }
  bool IsError() const noexcept { return !IsSuccess(); }

  template <typename T> Status &operator<<(const T &Part) &noexcept {
    Message_ << Part;
    return *this;
  }

  template <typename T> Status &&operator<<(const T &Part) &&noexcept {
    Message_ << Part;
    return std::move(*this);
  }

  static Status Error(ErrorCode Code, const std::string &Message = {}) noexcept {
    return Status{Code, Message};
  }

  static Status Success() { return Status{}; }

private:
  ErrorCode Code_;
  std::stringstream Message_;
};

template <typename T> class ErrorOr {
public:
  ErrorOr(T &&Value) noexcept { Storage_ = std::move(Value); }
  ErrorOr(Status &&Status) noexcept { Storage_ = std::move(Status); }

  ErrorOr(const ErrorOr &) noexcept = delete;
  ErrorOr(ErrorOr &&) noexcept = default;

  ErrorOr &operator=(const ErrorOr &) noexcept = delete;
  ErrorOr &operator=(ErrorOr &&) noexcept = default;

  ErrorCode Code() const noexcept {
    const auto *S = std::get_if<Status>(&Storage_);
    return S ? S->Code() : ErrorCode::Success;
  }

  const Status &GetError() noexcept {
    const auto *S = std::get_if<Status>(&Storage_);
    assert(S);
    return *S;
  }

  const T &GetValue() noexcept {
    const auto *V = std::get_if<T>(&Storage_);
    assert(V);
    return *V;
  }

  bool IsSuccess() const noexcept { return Code() == ErrorCode::Success; }
  bool IsError() const noexcept { return !IsSuccess(); }

private:
  std::variant<Status, T> Storage_;
};

} // namespace NotAGame

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>

namespace NotAGame {

struct NullIdT {};
inline constexpr NullIdT NullId;

template <typename T> class Id {
public:
  using value_type = uint32_t;

  static constexpr value_type INVALID_ID = std::numeric_limits<value_type>::max();

  Id() noexcept = default;
  Id(NullIdT) noexcept : Id() {}
  Id(const Id &RHS) noexcept = default;
  Id &operator=(const Id &RHS) noexcept = default;

  Id(value_type Value) noexcept : Value_{Value} {}
  Id(int Value) noexcept : Value_{static_cast<value_type>(Value)} {}
  Id(size_t Value) noexcept : Value_{static_cast<value_type>(Value)} {}

  operator value_type() const noexcept { return Value_; }
  value_type GetValue() const noexcept { return Value_; }

  bool IsValid() const noexcept { return Value_ != INVALID_ID; }
  bool IsInvalid() const noexcept { return !IsValid(); }

private:
  value_type Value_ = INVALID_ID;
};

} // namespace NotAGame

namespace std {

template <typename T> struct hash<NotAGame::Id<T>> {
  size_t operator()(NotAGame::Id<T> Id) const noexcept {
    return std::hash<typename NotAGame::Id<T>::value_type>{}(Id.GetValue());
  }
};

} // namespace std

#pragma once

#include "util/registry.h"
#include "util/types.h"

#include <boost/container/small_vector.hpp>

#include <string>

namespace NotAGame {

struct Resource {
  std::string Name;
  std::string Title;
  std::string Description;
};

class Resources {
public:
  using Amount = int;
  Resources(const Utils::Registry<Resource> &Registry) noexcept : Registry_{Registry} {
    Values_.resize(Registry.size(), 0);
  }

  Resources(const Utils::Registry<Resource> &Registry,
            std::initializer_list<Amount> Values) noexcept
      : Resources{Registry, Values.begin(), Values.end()} {}

  template <typename Iterable>
  Resources(const Utils::Registry<Resource> &Registry, const Iterable &Values) noexcept
      : Resources{Registry, Values.begin(), Values.end()} {}

  template <typename It>
  Resources(const Utils::Registry<Resource> &Registry, It Begin, It End) noexcept
      : Registry_{Registry} {
    Values_.resize(Registry.size());
    Values_.assign(Begin, End);
  }

  Resources(const Resources &RHS) noexcept = default;
  Resources(Resources &&RHS) noexcept = default;
  Resources &operator=(const Resources &RHS) noexcept {
    assert(&Registry_ == &RHS.Registry_);
    Values_ = RHS.Values_;
    return *this;
  }
  Resources &operator=(Resources &&RHS) noexcept {
    assert(&Registry_ == &RHS.Registry_);
    Values_ = std::move(RHS.Values_);
    return *this;
  };

  Resources operator+(const Resources &RHS) const noexcept {
    Resources Res{Registry_, 0};
    for (size_t I = 0, E = Values_.size(); I < E; ++I) {
      Res.Values_[I] = Values_[I] + RHS.Values_[I];
    }
    return Res;
  }

  Resources &operator+=(const Resources &RHS) noexcept {
    for (size_t I = 0, E = Values_.size(); I < E; ++I) {
      Values_[I] += RHS.Values_[I];
    }
    return *this;
  }

  Resources operator-(const Resources &RHS) const noexcept {
    Resources Res{Registry_, 0};
    for (size_t I = 0, E = Values_.size(); I < E; ++I) {
      Res.Values_[I] = Values_[I] - RHS.Values_[I];
    }
    return Res;
  }

  Resources &operator-=(const Resources &RHS) noexcept {
    for (size_t I = 0, E = Values_.size(); I < E; ++I) {
      Values_[I] -= RHS.Values_[I];
    }
    return *this;
  }

  Resources operator*(int RHS) const noexcept {
    Resources Res{Registry_, 0};
    for (size_t I = 0, E = Values_.size(); I < E; ++I) {
      Res.Values_[I] = Values_[I] * RHS;
    }
    return Res;
  }

  Resources &operator*=(int RHS) noexcept {
    for (size_t I = 0, E = Values_.size(); I < E; ++I) {
      Values_[I] *= RHS;
    }
    return *this;
  }

  Resources operator-() noexcept { return *this * -1; }

  bool operator==(const Resources &RHS) const noexcept {
    return Compare(RHS, [](const auto &LHS, const auto &RHS) { return LHS != RHS; });
  }

  bool operator!=(const Resources &RHS) const noexcept {
    return Compare(RHS, [](const auto &LHS, const auto &RHS) { return LHS == RHS; });
  }

  // Beware: the comparisons are not strict so (not 'A < B') does not imply 'B >= A'.
  // Example: {0,1} and {1,0}.
  bool operator<(const Resources &RHS) const noexcept {
    return Compare(RHS, [](const auto &LHS, const auto &RHS) { return LHS >= RHS; });
  }

  bool operator>(const Resources &RHS) const noexcept {
    return Compare(RHS, [](const auto &LHS, const auto &RHS) { return LHS <= RHS; });
  }

  bool operator<=(const Resources &RHS) const noexcept {
    return Compare(RHS, [](const auto &LHS, const auto &RHS) { return LHS > RHS; });
  }

  bool operator>=(const Resources &RHS) const noexcept {
    return Compare(RHS, [](const auto &LHS, const auto &RHS) { return LHS < RHS; });
  }

  Amount GetAmountByName(const std::string &Name) const noexcept {
    const auto ResId = Registry_.GetId(Name);
    return Values_[ResId];
  }

  Amount GetAmountById(Id<Resource> Id) const noexcept {
    assert(Id < Values_.size());
    return Values_[Id];
  }

  void SetAmountByName(const std::string &Name, Amount Amount) noexcept {
    const auto ResId = Registry_.GetId(Name);
    Values_[ResId] = Amount;
  }

  void SetAmountById(Id<Resource> Id, Amount Amount) noexcept {
    assert(Id < Values_.size());
    Values_[Id] = Amount;
  }

private:
  Resources(const Utils::Registry<Resource> &Registry, int) : Registry_{Registry} {
    Values_.resize(Registry.size());
  }

  template <typename TNegFn>
  bool Compare(const Resources &RHS, TNegFn &&Comparator) const noexcept {
    for (size_t I = 0, E = Values_.size(); I < E; ++I) {
      if (Comparator(Values_[I], RHS.Values_[I])) {
        return false;
      }
    }
    return true;
  }

  boost::container::small_vector<Amount, 8> Values_;
  const Utils::Registry<Resource> &Registry_;
};

} // namespace NotAGame

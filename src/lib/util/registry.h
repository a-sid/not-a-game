#pragma once

#include <unordered_map>

#include "util/assert.h"
#include "util/id.h"
#include "util/logger.h"
#include "util/map_utils.h"
#include "util/paged_vector.h"
#include "util/types.h"

namespace NotAGame::Utils {

template <typename T, size_t N = 64> class Registry {
public:
  using Key = std::string;

  const T &GetObjectByKey(const Key &Key) const noexcept {
    const auto *Object = TryGetObjectByKey(Key);
    if (!Object) {
      LogFatal() << "No object found with Key=" << Key;
    }
    return *Object;
  }

  const T &GetObjectById(Id<T> Id) const noexcept {
    const auto *Object = TryGetObjectById(Id);
    if (!Object) {
      LogFatal() << "No object found with Id=" << Id;
    }
    return *Object;
  }

  T &GetObjectById(Id<T> Id) noexcept {
    auto *Object = TryGetObjectById(Id);
    if (!Object) {
      LogFatal() << "No object found with Id=" << Id;
    }
    return *Object;
  }

  const T *TryGetObjectByKey(const Key &Key) const noexcept {
    auto Found = MapFindPtr(Objects_, Key);
    return Found ? &Storage_[*Found] : nullptr;
  }
  T *TryGetObjectByKey(const Key &Key) noexcept {
    auto Found = MapFindPtr(Objects_, Key);
    return Found ? &Storage_[*Found] : nullptr;
  }

  const T *TryGetObjectById(Id<T> Id) const noexcept {
    return Id < size() ? &Storage_[Id] : nullptr;
  }
  T *TryGetObjectById(Id<T> Id) noexcept { return Id < size() ? &Storage_[Id] : nullptr; }

  Id<T> GetId(const Key &Key) const noexcept {
    const auto *Id = TryGetId(Key);
    if (!Id) {
      LogFatal() << "No object found with Key=" << Key;
    }
    return *Id;
  }

  const Id<T> *TryGetId(const Key &Key) const noexcept { return MapFindPtr(Objects_, Key); }

  template <typename U> Id<T> AddObject(Key &&Key, U &&Object) noexcept {
    Storage_.emplace_back(std::forward<U>(Object));
    const Id<T> NewId = size();
    const auto [Iter, IsNew] = Objects_.emplace(std::move(Key), NewId);
    if (!IsNew) {
      LogFatal() << "Object with Key=" << Key << " already exists!";
    }
    return NewId;
  }

  const size_t size() const noexcept { return Objects_.size(); }
  const size_t empty() const noexcept { return Objects_.empty(); }

  using iterator = typename PagedVector<T, N>::iterator;
  using const_iterator = typename PagedVector<T, N>::const_iterator;

  iterator begin() noexcept { return Storage_.begin(); }
  iterator end() noexcept { return Storage_.end(); }

  const_iterator begin() const noexcept { return Storage_.begin(); }
  const_iterator end() const noexcept { return Storage_.end(); }

private:
  PagedVector<T, N> Storage_;
  std::unordered_map<Key, Id<T>> Objects_;
};

} // namespace NotAGame::Utils

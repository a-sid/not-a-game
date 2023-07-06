#pragma once

#include <algorithm>
#include <cassert>
#include <tuple>
#include <vector>

namespace NotAGame::Utils {

template <typename T, size_t N> class PagedVector {
public:
  static_assert(N != 0);

  explicit PagedVector(size_t NumPages = 1) noexcept {
    NumPages = std::max(NumPages, size_t{1});
    for (size_t I = 0; I < NumPages; ++I) {
      AllocatePage();
    }
  }

  PagedVector(const PagedVector &RHS) noexcept = default;
  PagedVector(PagedVector &&RHS) noexcept = default;
  PagedVector &operator=(const PagedVector &RHS) noexcept = default;
  PagedVector &operator=(PagedVector &&RHS) noexcept = default;
  ~PagedVector() noexcept = default;

  void push_back(const T &Element) noexcept { return GetOrAllocatePage().push_back(Element); }
  void push_back(T &&Element) noexcept { return GetOrAllocatePage().push_back(std::move(Element)); }

  template <typename... ArgsT> T &emplace_back(ArgsT &&...Args) noexcept {
    return GetOrAllocatePage().emplace_back(std::forward<ArgsT...>(Args...));
  }

  const T &operator[](size_t Idx) const noexcept {
    auto [PageNo, ElementIdx] = GetPageAndIndex(Idx);
    assert(PageNo <= LastPageIdx_ && ElementIdx < Storage_[PageNo].size());
    return Storage_[PageNo][ElementIdx];
  }

  T &operator[](size_t Idx) noexcept {
    auto [PageNo, ElementIdx] = GetPageAndIndex(Idx);
    assert(PageNo <= LastPageIdx_ && ElementIdx < Storage_[PageNo].size());
    return Storage_[PageNo][ElementIdx];
  }

  const size_t size() const noexcept { return LastPageIdx_ * N + Storage_[LastPageIdx_].size(); }
  const size_t capacity() const noexcept { return Storage_.size() * N; }
  const size_t empty() const noexcept { return size() == 0; }

private:
  using Page = std::vector<T>;

  void AllocatePage() noexcept {
    Page P;
    P.reserve(N);
    Storage_.push_back(std::move(P));
  }

  auto GetPageAndIndex(size_t Idx) const noexcept { return std::tuple{Idx / N, Idx % N}; }

  Page &GetOrAllocatePage() noexcept {
    if (Storage_[LastPageIdx_].size() == N) {
      ++LastPageIdx_;
      if (LastPageIdx_ == Storage_.size()) {
        AllocatePage();
      }
    }
    return Storage_[LastPageIdx_];
  }

private:
  std::vector<Page> Storage_;
  size_t LastPageIdx_ = 0;
};

} // namespace NotAGame::Utils

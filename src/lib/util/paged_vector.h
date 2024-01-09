#pragma once

#include <algorithm>
#include <cassert>
#include <tuple>
#include <vector>

namespace NotAGame::Utils {

template <typename T, size_t N> class PagedVector {
public:
  static_assert(N != 0);

  template <typename ParentT> class Iterator {
  public:
    typedef std::random_access_iterator_tag iterator_category;
    typedef T value_type;
    typedef T *pointer;
    typedef T &reference;
    typedef std::ptrdiff_t difference_type;

    explicit Iterator(ParentT *Parent = nullptr, size_t Idx = 0) noexcept
        : Parent_{Parent}, Idx_{Idx} {}

    Iterator &operator=(const Iterator &Rhs) noexcept = default;
    auto operator<=>(const Iterator &Rhs) const noexcept = default;

    Iterator &operator++() noexcept {
      ++Idx_;
      return *this;
    }

    Iterator &operator+=(difference_type n) noexcept {
      Idx_ += n;
      return *this;
    }

    Iterator operator+(difference_type n) noexcept { return Iterator{Parent_, Idx_ + n}; }

    Iterator &operator-=(difference_type n) noexcept {
      Idx_ -= n;
      return *this;
    }

    Iterator operator-(difference_type n) noexcept { return Iterator{Parent_, Idx_ - n}; }
    T &operator[](difference_type n) const noexcept { return *Iterator{Parent_, Idx_ + n}; }

    difference_type operator-(Iterator Rhs) const noexcept { return Idx_ - Rhs.Idx_; }

    Iterator operator++(int) noexcept {
      auto Iter = *this;
      ++(*this);
      return Iter;
    }

    const T &operator*() const noexcept {
      assert(Parent_);
      return (*Parent_)[Idx_];
    }

    T &operator*() noexcept {
      assert(Parent_);
      return (*Parent_)[Idx_];
    }

  private:
    auto AsPair() const noexcept { return std::pair{Parent_, Idx_}; }

    ParentT *Parent_ = nullptr;
    size_t Idx_ = 0;
  };

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

  const T &back() const noexcept {
    assert(!empty());
    return (*this)[size() - 1];
  }

  T &back() noexcept {
    assert(!empty());
    return (*this)[size() - 1];
  }

  const size_t size() const noexcept { return LastPageIdx_ * N + Storage_[LastPageIdx_].size(); }
  const size_t capacity() const noexcept { return Storage_.size() * N; }
  const size_t empty() const noexcept { return size() == 0; }

  using iterator = Iterator<PagedVector>;
  using const_iterator = Iterator<const PagedVector>;

  iterator begin() noexcept { return iterator{this, 0}; }
  iterator end() noexcept { return iterator{this, size()}; }

  const_iterator begin() const noexcept { return const_iterator{this, 0}; }
  const_iterator end() const noexcept { return const_iterator{this, size()}; }

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

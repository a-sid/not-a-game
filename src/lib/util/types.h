#pragma once

#include <cstdint>
#include <functional>
#include <limits>

#include <boost/container/small_vector.hpp>
#include <boost/container_hash/hash.hpp>

namespace NotAGame {

using Size = uint32_t;
using Dim = uint32_t;

constexpr Size MAX_SIZE = std::numeric_limits<Size>::max();

constexpr Size kMaxPlayers = 16;

struct Coord {
  Dim X;
  Dim Y;
};

struct Coord3D {
  Dim X;
  Dim Y;
  Dim Layer;

  auto operator<=>(const Coord3D &RHS) const noexcept = default;
};

struct Dims2D {
  Size Width;
  Size Height;
};

struct Dims3D {
  Size Width;
  Size Height;
  Size LayersCount;
};

inline constexpr Size Linearize(Coord C, Dims2D Size) noexcept { return C.X + Size.Width * C.Y; }

inline constexpr Size Linearize(Coord3D C, Dims3D Size) noexcept {
  return Size.Width * Size.Height * C.Layer + Size.Width * C.Y + C.X;
}

template <typename T, std::size_t N> using SmallVector = boost::container::small_vector<T, N>;

template <typename T> class IteratorRange {
public:
  IteratorRange(T Begin, T End) noexcept : Begin_{Begin}, End_{End} {}
  T begin() const noexcept { return Begin_; }
  T end() const noexcept { return End_; }

private:
  T Begin_;
  T End_;
};

template <typename T> IteratorRange<T> MakeRange(T Begin, T End) noexcept { return {Begin, End}; }

} // namespace NotAGame

namespace std {

template <> struct hash<NotAGame::Coord3D> {
  size_t operator()(const NotAGame::Coord3D &Coord) const noexcept {
    std::size_t Seed = 0;
    boost::hash_combine(Seed, Coord.X);
    boost::hash_combine(Seed, Coord.Y);
    boost::hash_combine(Seed, Coord.Layer);
    return Seed;
  }
};

} // namespace std

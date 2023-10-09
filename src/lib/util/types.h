#pragma once

#include <cstdint>
#include <limits>

#include <boost/container/small_vector.hpp>

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

} // namespace NotAGame

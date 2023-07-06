#pragma once

#include <cstdint>
#include <limits>

#include <boost/container/small_vector.hpp>

namespace NotAGame {

using Size = uint32_t;
using Dim = uint32_t;

constexpr Size MAX_SIZE = std::numeric_limits<Size>::max();

struct Coord {
  Dim X;
  Dim Y;
};

template <typename T, std::size_t N> using SmallVector = boost::container::small_vector<T, N>;

} // namespace NotAGame

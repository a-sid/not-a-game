#pragma once

#include "entities/global_map.h"
#include "entities/unit.h"
#include "util/types.h"

#include <cstdint>
#include <deque>
#include <string>

namespace NotAGame {

class Player {};

class GameState {
public:
  static constexpr Size MAX_PLAYERS = 64;

private:
  using Players = SmallVector<Player, MAX_PLAYERS>;

  GlobalMap GlobalMap_;
  std::deque<Unit> Units_;
  std::deque<Item> Items_;

  Players Players_;
  Players::iterator CurrentPlayer_;
  Size Turn_ = 0;
};

} // namespace NotAGame

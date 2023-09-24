#pragma once

#include "entities/unit.h"
#include "game/map.h"
#include "util/types.h"

#include "engine/player.h"
#include "engine/player_state.h"

#include <cstdint>
#include <deque>
#include <string>

namespace NotAGame {

struct PlayerAndState {
  Player Player;
  Size MapNumber;
};

class GameState {
public:
private:
  using Players = SmallVector<PlayerAndState, kMaxPlayers>;
  std::unordered_map<Size, Id<Player>> MapIndexToPlayer_;
  std::unordered_map<Id<Player>, Size> PlayerToMapIndex_;

  GlobalMap GlobalMap_;
  std::deque<Unit> Units_;
  std::deque<Item> Items_;

  Players Players_;
  Players::iterator CurrentPlayer_;
  Size Turn_ = 0;
};

} // namespace NotAGame

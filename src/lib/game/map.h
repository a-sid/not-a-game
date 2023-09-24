#pragma once

#include "engine/player.h"
#include "engine/player_state.h"
#include "entities/global_map.h"
#include "util/types.h"

namespace NotAGame {

class Map {

public:
private:
  GlobalMap GlobalMap_;

  SmallVector<PlayerState, kMaxPlayers> PlayerStates_;
};

} // namespace NotAGame

#pragma once

#include "entities/fraction.h"
#include "entities/global_map.h"
#include "entities/squad.h"
#include "entities/unit.h"
#include "util/types.h"

#include <string>
#include <unordered_set>

namespace NotAGame {

class PlayerState {

public:
private:
  GlobalMap MapView_;
  Id<MapObject> Capital_;
  std::unordered_set<Id<MapObject>> Towns_;
  std::unordered_set<Id<Spell>> Spells_;
  std::unordered_set<Id<Building>> Buildings_;
  SmallVector<Id<Squad>, 64> Squads_;
};

} // namespace NotAGame

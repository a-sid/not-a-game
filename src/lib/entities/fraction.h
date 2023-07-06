#pragma once

#include "entities/building.h"
#include "entities/spell.h"
#include "entities/unit.h"
#include "util/types.h"

#include <string>
#include <vector>

namespace NotAGame {

class Fraction {
public:
private:
  std::vector<Id<Spell>> Spells_;
  std::vector<Id<Unit>> UnitDescriptors_;
  std::vector<Id<Unit>> LeaderDescriptors_;
  std::vector<Id<Building>> Buildings_;
};

} // namespace NotAGame

#pragma once

#include "entities/building.h"
#include "entities/effect.h"
#include "util/id.h"
#include "util/types.h"

#include <string>

namespace NotAGame {

class Lord {

private:
  std::string Name_;
  std::string Description_;
  SmallVector<Id<Building>, 8> StartBuildings_;
  bool SpecialThiefOpsAllowed_ = false;
  Size TownUpgradeCostBonus = 0;
  float SpellLearnCostBonus = 0;
  Size SpellUsePerTurn_ = 1;
  Size RegenerationBonus = 10;
};

} // namespace NotAGame

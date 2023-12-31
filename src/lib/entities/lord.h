#pragma once

#include "entities/building.h"
#include "entities/effect.h"
#include "util/id.h"
#include "util/types.h"

#include <string>

namespace NotAGame {

class Lord : public Named {
  explicit Lord(Named &&Name) noexcept : Named{std::move(Name)} {}

private:
  SmallVector<Id<Building>, 8> StartBuildings_;
  bool SpecialThiefOpsAllowed_ = false;
  Size TownUpgradeCostBonus = 0;
  float SpellLearnCostBonus = 0;
  Size SpellUsePerTurn_ = 1;
  Size RegenerationBonus = 10;
};

} // namespace NotAGame

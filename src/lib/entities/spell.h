#pragma once

#include "entities/building.h"
#include "entities/resource.h"
#include "entities/unit.h"
#include "util/registry.h"
#include "util/types.h"

#include <deque>

namespace NotAGame {

struct Spell : public Named {
  Spell(Named Name, const ResourceRegistry &ResourceRegistry) noexcept
      : Named{std::move(Name)},
        LearningCost{ResourceRegistry}, UseCost{ResourceRegistry}, TradeCost{ResourceRegistry} {}

  Size Level;
  Effect SpellEffect;

  Resources LearningCost;
  Resources UseCost;
  Resources TradeCost;
};

} // namespace NotAGame

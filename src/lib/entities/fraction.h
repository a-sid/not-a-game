#pragma once

#include "entities/building.h"
#include "entities/common.h"
#include "entities/spell.h"
#include "entities/unit.h"
#include "util/registry.h"
#include "util/types.h"

#include <vector>

namespace NotAGame {

struct Fraction : public Named {
  Fraction(Named &&Name) noexcept : Named{std::move(Name)} {}

  std::vector<Id<Spell>> Spells;
  std::vector<Id<Unit>> Units;
  std::vector<Id<Unit>> Leaders;
  Utils::Registry<Building> Buildings;
};

} // namespace NotAGame

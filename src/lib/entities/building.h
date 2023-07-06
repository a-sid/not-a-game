#pragma once

#include "util/id.h"
#include "util/types.h"

#include <vector>

namespace NotAGame {

class Building;

class Fraction;
class UnitDescriptor;
class Icon;

using BuildingId = Id<Building>;

class Building {
private:
  std::vector<BuildingId> Dependencies_;
  Id<Fraction> Fraction_;
  Id<UnitDescriptor> UnitUnblocked_;
  Id<Icon> Icon_;
  Id<Icon> Picture_;
};

} // namespace NotAGame

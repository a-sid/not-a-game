#pragma once

#include "entities/common.h"
#include "entities/resource.h"

#include "util/id.h"
#include "util/types.h"

#include <vector>

namespace NotAGame {

struct Building;

struct UnitDescriptor;
class Icon;

using BuildingId = Id<Building>;

struct Building : public Named {
  Building(Named &&Name, const ResourceRegistry &Resources) noexcept
      : Named{std::move(Name)}, Cost{Resources} {}
  SmallVector<BuildingId, 4> Requirements;
  std::string FunctionalDescription;
  Id<UnitDescriptor> UnitUnblocked;
  Resources Cost;
  Id<Icon> BulidingIcon;
  Id<Icon> Picture;
};

} // namespace NotAGame

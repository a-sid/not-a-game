#pragma once

#include "entities/components.h"
#include "entities/global_map.h"
#include "util/types.h"

#include <vector>

namespace NotAGame {

struct Waypoint {
  Coord3D Coord;
  Size Cost;
  Size DayIndex;
};

struct Path {
  Coord3D Start;
  Coord3D Target;
  std::vector<Waypoint> Waypoints;
};

std::optional<Path> TryBuildPath(Coord3D From, Coord3D To, const GlobalMap &Map,
                                 const GameplaySystems &Systems, const Mod &M,
                                 const Id<Unit> LeaderId) noexcept;
} // namespace NotAGame

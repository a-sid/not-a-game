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

Size ComputeMoveCost(Coord3D From, Coord3D To, const GlobalMap &Map, const Mod &M) noexcept;

struct Path {
  Coord3D Start;
  Coord3D Target;
  std::vector<Waypoint> Waypoints;

  void Walk(size_t NumSteps) noexcept {
    assert(NumSteps < Waypoints.size() - 1);
    Waypoints.erase(Waypoints.begin(), Waypoints.begin() + NumSteps);
    Start = Waypoints[0].Coord;
    auto BaseCost = Waypoints[0].Cost;
    for (auto &Pt : Waypoints) {
      Pt.Cost -= BaseCost;
    }
  }
};

std::optional<Path> TryBuildPath(Coord3D From, Coord3D To, const GlobalMap &Map,
                                 const GameplaySystems &Systems, const Mod &M,
                                 const Id<Unit> LeaderId) noexcept;
} // namespace NotAGame

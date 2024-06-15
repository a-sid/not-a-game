#pragma once

#include "entities/unit_grid.h"
#include "util/id.h"
#include "util/registry.h"
#include "util/types.h"

#include <cstdint>
#include <string>

namespace NotAGame {

struct GuardComponent;
class Player;
class Unit;

class Squad {
public:
  Squad(const GridSettings &Settings, Id<Unit> Leader, Id<Player> PlayerId) noexcept
      : Grid_{Settings.Width, Settings.Height}, Leader_{Leader}, Player_{PlayerId} {
    Units.push_back(Leader);
  }
  Grid &GetGrid() noexcept { return Grid_; }

  Id<Unit> GetUnit(Dim X, Dim Y) const noexcept { return Grid_.GetUnit(X, Y); }
  Id<Unit> GetLeader() const noexcept { return Leader_; }

  Id<Squad> ComponentId;
  Id<GuardComponent> GuardId;
  Id<Player> Player_;
  Coord3D Position;
  SmallVector<Id<Unit>, 8> Units;

private:
  Grid Grid_;
  Id<Unit> Leader_;
};

} // namespace NotAGame

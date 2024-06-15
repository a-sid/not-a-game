#include "entities/unit.h"

#include "entities/components.h"
#include "entities/squad.h"

namespace NotAGame {

ReachableUnits SquadRange::ComputeReachableUnits(GameplaySystems &Systems, Squad &Attacker,
                                                 Squad &Defender, const Unit &U) const noexcept {
  auto &S = U.SquadId == Attacker.ComponentId ? Attacker : Defender;
  return ComputeReachableUnitsInSquad(Systems, S, U);
}

ReachableUnits NearestUnitRange::ComputeReachableUnits(GameplaySystems &Systems, Squad &EnemySquad,
                                                       Squad &FriendSquad,
                                                       const Unit &U) const noexcept {
  ReachableUnits Result;
  // Do we have any friendly units before us?
  for (auto FriendId : FriendSquad.Units) {
    Unit &Friend = Systems.Units.GetComponent(FriendId);
    if (Friend.IsAlive() && Friend.GridPosition.X < U.GridPosition.X) {
      return Result;
    }
  }

  // Collect units from the nearest line.
  Size NearestColumn = GridWidth_;
  for (auto EnemyId : EnemySquad.Units) {
    auto &Enemy = Systems.Units.GetComponent(EnemyId);
    if (Enemy.GridPosition.X < NearestColumn) {
      NearestColumn = Enemy.GridPosition.X;
      Result.clear();
      Result.push_back(&Enemy);
    } else if (Enemy.GridPosition.X == NearestColumn) {
      Result.push_back(&Enemy);
    }
  }

  // Leave only reachable units.
  auto YDistance = [&U](const auto &Enemy) {
    return std::abs(static_cast<int32_t>(Enemy.GridPosition.Y) -
                    static_cast<int32_t>(U.GridPosition.Y));
  };
  std::sort(Result.begin(), Result.end(), [&YDistance](const auto &LHS, const auto &RHS) {
    return YDistance(*LHS) < YDistance(*RHS);
  });
  if (YDistance(*Result.front()) > 1) {
    Result.resize(1);
  } else {
    auto It = std::find_if(Result.begin(), Result.end(),
                           [YDistance](const auto *U) { return YDistance(*U) > 1; });
    Result.resize(std::distance(Result.begin(), It));
  }

  return Result;
}

ReachableUnits AnyUnitRange::ComputeReachableUnitsInSquad(GameplaySystems &Systems, Squad &S,
                                                          const Unit &U) const noexcept {
  ReachableUnits Result;
  for (auto Id : S.Units) {
    auto &Target = Systems.Units.GetComponent(Id);
    Result.push_back(&Target);
  }
  return Result;
}

ReachableUnits AllUnitRange::ComputeReachableUnitsInSquad(GameplaySystems &Systems, Squad &S,
                                                          const Unit &U) const noexcept {
  ReachableUnits Result;
  for (auto Id : S.Units) {
    auto &Target = Systems.Units.GetComponent(Id);
    Result.push_back(&Target);
  }
  return Result;
}

} // namespace NotAGame

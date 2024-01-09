#pragma once

#include "entities/fraction.h"
#include "entities/squad.h"

#include "util/id.h"
#include "util/types.h"

#include <memory>

namespace NotAGame {

class Player;
using PlayerId = Id<Player>;

class MapObject;
using MapObjectId = Id<MapObject>;
using MapObjectPtr = std::unique_ptr<MapObject>;

struct MapComponent {
  MapObjectId ObjectId;
};

struct GarrisonComponent : public MapComponent {
  Grid Garrison;
  PlayerId Owner;
};

struct GuardComponent : public MapComponent {
  Id<GuardComponent> ComponentId;
  Id<Squad> SquadId;
  PlayerId Owner;
};

struct TownComponent : public MapComponent {
  Size Level;
  Id<GuardComponent> GuardId;
  Id<GarrisonComponent> GarrisonId;
  PlayerId Owner;
};

struct CapitalComponent : public MapComponent {
  Id<CapitalComponent> ComponentId;
  Id<Fraction> FractionId;
  Id<GuardComponent> GuardId;
  Id<GarrisonComponent> GarrisonId;
  Id<Player> PlayerId;
};

struct RewardComponent : public MapComponent {
  std::optional<Resources> Resource;
  Id<Inventory> Items;
};

struct BattleResult {
  std::string Loser;
  std::string Winner;
};

using BattleResults = std::vector<BattleResult>;

struct GraveComponent : public MapComponent {
  std::vector<BattleResults> Battles;
};

} // namespace NotAGame

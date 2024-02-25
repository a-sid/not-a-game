#pragma once

#include "entities/global_map.h"
#include "entities/resource.h"
#include "entities/squad.h"
#include "util/id.h"
#include "util/paged_vector.h"
#include "util/types.h"

#include <optional>
#include <ranges>
#include <unordered_set>

namespace NotAGame {

class Player;
using PlayerId = Id<Player>;

class MapObject;
using MapObjectId = Id<MapObject>;

struct LandPropagationSettings {
  SmallVector<Size, 8> TownPropagationByLevel;
  Size CapitalPropagation;
  Size RodPropagation;
};

struct LandPropagation {
  Id<LandPropagation> ComponentId;
  PlayerId Player;
  Size TilesPerTurn;
  Coord3D Origin;
  Dims2D OriginSize;
};

template <typename T, size_t PageSize = 64> class GameplaySystem {
public:
  using TurnableComponent = std::optional<T>;

  T &AddComponent(T &&Component) noexcept {
    const auto ComponentId = Components_.size();
    auto &NewComponent = Components_.emplace_back(std::move(Component));
    NewComponent->ComponentId = ComponentId;
    return *NewComponent;
  }

  T &AddComponent(const T &Component) noexcept {
    const auto ComponentId = Components_.size();
    auto &NewComponent = Components_.emplace_back(Component);
    NewComponent->ComponentId = ComponentId;
    return *NewComponent;
  }

  void RemoveComponent(Id<T> ComponentId) noexcept { Components_[ComponentId].reset(); }

  const T &GetComponent(Id<T> ComponentId) const noexcept {
    const auto &Value = Components_[ComponentId];
    assert(Value);
    return *Value;
  }

  T &GetComponent(Id<T> ComponentId) noexcept {
    auto &Value = Components_[ComponentId];
    assert(Value);
    return *Value;
  }

  T *GetComponentOrNull(Id<T> ComponentId) noexcept {
    return ComponentId.IsValid() ? &GetComponent(ComponentId) : nullptr;
  }

private:
  Utils::PagedVector<TurnableComponent, PageSize> Components_;
};

class GlobalMap;

template <typename T, size_t PageSize = 64>
class ByPlayerSystem : private GameplaySystem<T, PageSize> {
public:
  using Parent = GameplaySystem<T>;
  using IdsByPlayer = std::unordered_set<Id<T>>;
  using Parent::GetComponent;

  explicit ByPlayerSystem(Size NumPlayers) noexcept { ComponentsByPlayer_.resize(NumPlayers); }

  T &AddComponent(const T &Component) noexcept {
    auto PlayerId = Component.Player;
    auto &NewComponent = Parent::AddComponent(Component);
    ComponentsByPlayer_[PlayerId].insert(NewComponent.ComponentId);
    return NewComponent;
  }

  void RemoveComponent(Id<T> ComponentId) noexcept {
    const auto &Component = GetComponent(ComponentId);
    ComponentsByPlayer_[Component.Player].erase(ComponentId);
    Parent::RemoveComponent(ComponentId);
  }

  void ChangeOwner(Id<T> ComponentId, PlayerId PlayerId) noexcept {
    const auto &Component = GetComponent(ComponentId);
    ComponentsByPlayer_[Component.Player].erase(ComponentId);
    ComponentsByPlayer_[PlayerId].insert(ComponentId);
  }

  const IdsByPlayer &GetComponentsByPlayer(PlayerId PlayerId) const noexcept {
    return ComponentsByPlayer_[PlayerId];
  }

  IdsByPlayer &GetComponentsByPlayer(PlayerId PlayerId) noexcept {
    return ComponentsByPlayer_[PlayerId];
  }

private:
  SmallVector<IdsByPlayer, kMaxPlayers> ComponentsByPlayer_;
};

struct PropagationResult {
  Id<Player> PlayerId;
  // Propagation is only done on single layer.
  Dim Layer;
  std::vector<Coord> Tiles;
};

class LandPropagationSystem : public ByPlayerSystem<LandPropagation> {
public:
  using Parent = ByPlayerSystem<LandPropagation>;
  explicit LandPropagationSystem(Size PlayersCount) noexcept : Parent{PlayersCount} {}
  PropagationResult AddComponentAndPropagate(GlobalMap &Map,
                                             const LandPropagation &Component) noexcept {
    Parent::AddComponent(Component);
    return Propagate(Map, Component);
  }

  std::vector<Coord3D> Propagate(GlobalMap &Map, PlayerId Player) noexcept { return {}; } // FIXME
  PropagationResult Propagate(GlobalMap &Map, const LandPropagation &Component) noexcept;
};

struct VisibilityRangeSettings {
  SmallVector<Size, 8> TownVisibilityByLevel;
  Size CapitalVisibility;
};

struct VisibilityRange {
  Id<VisibilityRange> ComponentId;
  PlayerId Player;
  Coord3D Origin;
  Dims2D OriginSize;
  Size Radius;
};

class VisibilitySystem : public ByPlayerSystem<VisibilityRange> {
public:
  using Parent = ByPlayerSystem<VisibilityRange>;
  using Visibility = std::vector<bool>;
  using VisibilityDiff = std::vector<Coord3D>;

  VisibilitySystem(Size PlayersCount, Dims3D MapSize) noexcept
      : Parent{PlayersCount}, MapSize_{MapSize} {
    PlayerVisibilities_.resize(PlayersCount);
    for (Size I = 0; I < PlayersCount; ++I) {
      PlayerVisibilities_[I].resize(MapSize.Height * MapSize.Width * MapSize.LayersCount, false);
      Reset(PlayerId{I});
    }
  }

  void Reset(PlayerId Player) noexcept {
    auto &Visibility = PlayerVisibilities_[Player];
    std::vector<Coord3D> Diff;

    std::fill(Visibility.begin(), Visibility.end(), false);
    for (auto ComponentId : GetComponentsByPlayer(Player)) {
      RenderVisibility(Diff, GetComponent(ComponentId));
    }
  }

  void MoveComponent(Id<VisibilityRange> ComponentId, Coord3D Coord) noexcept {
    VisibilityDiff Diff;
    auto &Component = GetComponent(ComponentId);
    Component.Origin = Coord;
    RenderVisibility(Diff, Component);
  }

  VisibilityDiff HideRegion(PlayerId PlayerId, Dim Layer, Dim X, Dim Y, Dim Width,
                            Dim Height) noexcept {
    VisibilityDiff Diff;
    SetAndDiffRegionVisibility(Diff, PlayerId, Layer, X, Y, Width, Height, false);
    return Diff;
    // TODO: consider always visible regions
  }

  void SetAndDiffRegionVisibility(VisibilityDiff &Diff, PlayerId PlayerId, Dim Layer, Dim X, Dim Y,
                                  Dim Width, Dim Height, bool NewValue) noexcept {
    auto &Visibility = PlayerVisibilities_[PlayerId];
    for (Size IW = X; IW < X + Width; ++IW) {
      for (Size IH = Y; IH < Y + Height; ++IH) {
        const Coord3D Coord{.X = IW, .Y = IH, .Layer = Layer};
        const auto Offset = Linearize(Coord, MapSize_);
        if (Visibility[Offset] != NewValue) {
          Diff.push_back(Coord);
          Visibility[Offset] = NewValue;
        }
      }
    }
  }

  void SetRegionVisibility(PlayerId PlayerId, Dim Layer, Dim X, Dim Y, Dim Width, Dim Height,
                           bool NewValue) noexcept {
    auto &Visibility = PlayerVisibilities_[PlayerId];
    for (Size IW = X; IW < X + Width; ++IW) {
      for (Size IH = Y; IH < Y + Height; ++IH) {
        const Coord3D Coord{.X = IW, .Y = IH, .Layer = Layer};
        const auto Offset = Linearize(Coord, MapSize_);
        Visibility[Offset] = NewValue;
      }
    }
  }

private:
  void RenderVisibility(VisibilityDiff &Diff, const VisibilityRange &Component) noexcept {
    Size WidthFrom = std::min(Component.Origin.X, Component.Radius);
    Size WidthTo = std::min(Component.Origin.X + Component.OriginSize.Width + Component.Radius,
                            MapSize_.Width);
    Size HeightFrom = std::min(Component.Origin.Y, Component.Radius);
    Size HeightTo = std::min(Component.Origin.Y + Component.OriginSize.Height + Component.Radius,
                             MapSize_.Height);
    SetAndDiffRegionVisibility(Diff, Component.Player, Component.Origin.Layer,
                               Component.Origin.X - WidthFrom, Component.Origin.Y - HeightFrom,
                               WidthFrom + WidthTo, HeightFrom - HeightTo, true);
  }

  SmallVector<std::vector<bool>, kMaxPlayers> PlayerVisibilities_;
  Dims3D MapSize_;
};

struct ResourceSource {
  Id<ResourceSource> ComponentId;
  Resources Income;
  PlayerId Player;
};

class ResourceSystem : public ByPlayerSystem<ResourceSource> {
public:
  using Parent = ByPlayerSystem<ResourceSource>;
  ResourceSystem(Size PlayersCount, const ResourceRegistry &ResourceRegistry) noexcept
      : Parent{PlayersCount}, ResourceRegistry_{ResourceRegistry} {}

  Resources GetTotalIncome(PlayerId PlayerId) const noexcept {
    const auto &AllComponents = GetComponentsByPlayer(PlayerId);
    Resources Result{ResourceRegistry_};
    for (const auto &ComponentId : AllComponents) {
      Result += GetComponent(ComponentId).Income;
    }
    return Result;
  }

private:
  const ResourceRegistry &ResourceRegistry_;
};

class MapObject;
using MapObjectId = Id<MapObject>;

using GarrisonSystem = GameplaySystem<GarrisonComponent>;

using GuardSystem = GameplaySystem<GuardComponent>;

using UnitSystem = GameplaySystem<Unit, 1024>;
using LeaderSystem = GameplaySystem<LeaderData>;
using SquadSystem = GameplaySystem<Squad>;

using CapitalSystem = GameplaySystem<CapitalComponent>;
using TownSystem = GameplaySystem<TownComponent>;

struct GameplaySystems {
  GameplaySystems(const ResourceRegistry &ResourceRegistry, Size PlayersCount,
                  Dims3D MapSize) noexcept
      : LandPropagation{PlayersCount}, Visibility{PlayersCount, MapSize}, Resources{
                                                                              PlayersCount,
                                                                              ResourceRegistry} {}
  LandPropagationSystem LandPropagation;
  VisibilitySystem Visibility;
  ResourceSystem Resources;
  UnitSystem Units;
  LeaderSystem Leaders;
  GuardSystem Guards;
  GarrisonSystem Garrisons;
  SquadSystem Squads;
  CapitalSystem Capitals;
  TownSystem Towns;
};

} // namespace NotAGame

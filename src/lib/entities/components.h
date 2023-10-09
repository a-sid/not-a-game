#pragma once

#include "util/id.h"
#include "util/paged_vector.h"
#include "util/types.h"

#include <optional>
#include <unordered_set>

namespace NotAGame {

class Player;

using PlayerId = Id<Player>;

struct LandPropagationSettings {
  SmallVector<Size, 8> TownPropagationByLevel;
  Size CapitalPropagation;
  Size RodPropagation;
};

struct LandPropagation {
  Id<LandPropagation> ComponentId;
  PlayerId Player;
  Size TilesPerTurn;
  Coord Origin;
  Dims2D OriginSize;
};

template <typename T, size_t PageSize = 64> class GameplaySystem {
public:
  using TurnableComponent = std::optional<T>;
  Id<T> AddComponent(T &&Component) noexcept {
    Components_.push_back(TurnableComponent{std::move(Component)});
    return Components_.size() - 1;
  }
  Id<T> AddComponent(const T &Component) noexcept {
    Components_.push_back(TurnableComponent{Component});
    const auto ComponentId = Components_.size() - 1;
    Components_.back()->ComponentId = ComponentId;
    return ComponentId;
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

  Id<T> AddComponent(const T &Component) noexcept {
    auto PlayerId = Component.Player;
    auto ComponentId = Parent::AddComponent(Component);
    ComponentsByPlayer_[PlayerId].insert(ComponentId);
    return ComponentId;
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

class LandPropagationSystem : public ByPlayerSystem<LandPropagation> {
public:
  using Parent = ByPlayerSystem<LandPropagation>;
  explicit LandPropagationSystem(Size PlayersCount) noexcept : Parent{PlayersCount} {}

  void Propagate(GlobalMap &Map, PlayerId Player) noexcept {}
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

  VisibilitySystem(Size PlayersCount, Dims3D MapSize) noexcept
      : Parent{PlayersCount}, MapSize_{MapSize} {
    PlayerVisibilities_.resize(PlayersCount);
    for (Size I = 0; I < PlayersCount; ++I) {
      PlayerVisibilities_[I].resize(MapSize.Height * MapSize.Width * MapSize.LayersCount);
      Reset(PlayerId{I});
    }
  }

  void Reset(PlayerId Player) noexcept {
    auto &Visibility = PlayerVisibilities_[Player];
    std::fill(Visibility.begin(), Visibility.end(), false);
    for (const auto &ComponentId : GetComponentsByPlayer(Player)) {
      RenderVisibility(GetComponent(ComponentId));
    }
  }

  void MoveComponent(Id<VisibilityRange> ComponentId, Coord3D Coord) noexcept {
    auto &Component = GetComponent(ComponentId);
    Component.Origin = Coord;
    RenderVisibility(Component);
  }

private:
  void RenderVisibility(const VisibilityRange &Component) noexcept {
    Size WidthFrom = std::min(Component.Origin.X, Component.Radius);
    Size WidthTo = std::min(Component.Origin.X + Component.OriginSize.Width + Component.Radius,
                            MapSize_.Width);
    Size HeightFrom = std::min(Component.Origin.Y, Component.Radius);
    Size HeightTo = std::min(Component.Origin.Y + Component.OriginSize.Height + Component.Radius,
                             MapSize_.Height);
    auto &Visibility = PlayerVisibilities_[Component.Player];
    for (Size IW = Component.Origin.X - WidthFrom; IW < WidthTo; ++IW) {
      for (Size IH = Component.Origin.Y - HeightFrom; IH < HeightTo; ++IH) {
        Visibility[Linearize(Coord3D{.X = IW, .Y = IH, .Layer = Component.Origin.Layer},
                             MapSize_)] = true;
      }
    }
  }

  SmallVector<std::vector<bool>, kMaxPlayers> PlayerVisibilities_;
  Dims3D MapSize_;
};

struct GameplaySystems {
  LandPropagationSystem LandPropagation;
  VisibilitySystem Visibility;
};

} // namespace NotAGame

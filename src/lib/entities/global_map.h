#pragma once

#include "entities/common.h"
#include "entities/inventory.h"
#include "entities/map_components.h"
#include "entities/squad.h"
#include "status/status.h"
#include "util/id.h"
#include "util/registry.h"
#include "util/types.h"

#include <cassert>
#include <deque>
#include <memory>
#include <optional>
#include <span>
#include <vector>

namespace NotAGame {

class Animation;
class Fraction;
class Mod;
class Player;
class Squad;

struct LandPropagation;
struct VisibilityRange;
struct ResourceSource;

enum class TerrainKind { Ground, Water, Relief };

enum class GroundKind { Plain, Road, Forest };

class MapObject;
using MapObjectId = Id<MapObject>;
using MapObjectPtr = std::unique_ptr<MapObject>;

class MapObject : public Named {
public:
  enum Kind {
    Capital,
    Town,
    Grave, // TODO: make it a terrain trait.
    Rod,
    Shop,
    MagicShop,
    TrainingCamp,
    Resource,
    Ruins,
    Squad,
    Other,
  };

  MapObject(Named Name, Kind Kind, Coord3D Pos, Dims2D Size, std::optional<Coord> EntrancePos,
            bool IsPassable, bool IsLandPropagationBarrier) noexcept
      : Named{std::move(Name)}, Kind_{Kind}, Pos_{Pos}, Size_{Size}, EntrancePos_{EntrancePos},
        IsPassable_{IsPassable}, IsLandPropagationBarrier_{IsLandPropagationBarrier} {}

  Kind GetKind() const noexcept { return Kind_; }
  Coord3D GetPosition() const noexcept { return Pos_; }
  Dim GetLayer() const noexcept { return Pos_.Layer; }
  Dim GetX() const noexcept { return Pos_.X; }
  Dim GetY() const noexcept { return Pos_.Y; }

  Dims2D GetSize() const noexcept { return Size_; }
  Size GetWidth() const noexcept { return Size_.Width; }
  Size GetHeight() const noexcept { return Size_.Height; }

  std::optional<Coord> GetEntrancePos() const noexcept { return EntrancePos_; }

  // void Register(const Mod &, GameplaySystems &) noexcept {}

  Id<Player> Owner;
  Id<NotAGame::Squad> SquadTrait;
  Id<GarrisonComponent> Garrison;
  Id<GuardComponent> Guard;
  Id<CapitalComponent> CapitalTrait;
  Id<TownComponent> TownTrait;
  Id<GraveComponent> GraveTrait;
  Id<Inventory> InventoryTrait;
  Id<LandPropagation> LandPropagationTrait;
  Id<VisibilityRange> VisibilityRangeTrait;
  Id<ResourceSource> ResourceTrait;
  Id<RewardComponent> RewardTrait;

protected:
  Kind Kind_;
  Id<Animation> AnimationId_;
  Coord3D Pos_;
  Dims2D Size_;
  std::optional<Coord> EntrancePos_;
  bool IsPassable_;
  bool IsLandPropagationBarrier_;
  bool IsEnterable_;
};

class Terrain : public Named {
public:
  Terrain(Named Name, Size BaseCost) noexcept : Named{std::move(Name)}, BaseCost_{BaseCost} {}
  Size GetBaseCost() const noexcept { return BaseCost_; }

private:
  Size BaseCost_;
};

class Tile {
public:
  explicit Tile(Id<Terrain> Terrain = NullId) noexcept : Terrain_{Terrain} {}
  Id<Terrain> Terrain_;
  Id<Player> Owner_;
  // std::optional<Grave> Grave_;
  Id<MapObject> Object_ = NullId;
  Id<Squad> Squad_ = NullId;
  // By default, all tiles are visible to the neutral player which has index 0.
  uint16_t VisibilityFlags = 1;
};

class GlobalMap {
public:
  GlobalMap(Size Layers, Size Width, Size Height) noexcept
      : Width_{Width}, Height_{Height}, Layers_{Layers} {
    Tiles_.resize(Width * Height * Layers);
    ObjectsByLayer_.resize(Layers);
  }

  Size GetWidth() const noexcept { return Width_; }
  Size GetHeight() const noexcept { return Height_; }
  Size GetNumLayers() const noexcept { return Layers_; }

  Dims3D GetSize() const noexcept {
    return Dims3D{.Width = Width_, .Height = Height_, .LayersCount = Layers_};
  }

  ErrorOr<MapObjectId> AddObject(std::string Name, MapObject Object) noexcept {
    assert(Object.GetHeight() && Object.GetWidth());
    auto MaxX = Object.GetX() + Object.GetWidth();
    auto MaxY = Object.GetY() + Object.GetHeight();

    if (MaxX > Width_ || MaxY > Height_) {
      return Status::Error(ErrorCode::MapError, "Coordinates out of bounds");
    }

    if (!CanPlaceObject(Object.GetLayer(), Object.GetX(), Object.GetY(), Object)) {
      return Status::Error(ErrorCode::MapError, "Object overlap is not allowed");
    }

    const bool IsCapital = Object.GetKind() == MapObject::Kind::Capital;
    auto Id = MapObjects_.AddObject(std::move(Name), std::move(Object));
    ObjectsByLayer_[Object.GetLayer()].push_back(Id);
    for (Dim X = Object.GetX(); X < MaxX; ++X) {
      for (Dim Y = Object.GetY(); Y < MaxY; ++Y) {
        GetTile(Object.GetLayer(), X, Y).Object_ = Id;
      }
    }

    return Id;
  }

  bool CanPlaceObject(Dim Layer, Dim X, Dim Y, const MapObject &Object) const noexcept {
    auto MaxX = X + Object.GetHeight();
    auto MaxY = Y + Object.GetWidth();

    for (Dim x = X; x < MaxX; ++x) {
      for (Dim y = Y; y < MaxY; ++y) {
        const auto &Tile = GetTile(Layer, x, y);
        if (Tile.Object_.IsValid()) {
          return false;
        }
      }
    }
    return true;
  }

  Tile &GetTile(Dim Layer, Dim X, Dim Y) noexcept {
    return Tiles_[Width_ * Height_ * Layer + Y * Width_ + X];
  }
  const Tile &GetTile(Dim Layer, Dim X, Dim Y) const noexcept {
    return Tiles_[Width_ * Height_ * Layer + Y * Width_ + X];
  }

  auto GetObjects() noexcept { return MakeRange(MapObjects_.begin(), MapObjects_.end()); }
  auto GetObjects() const noexcept { return MakeRange(MapObjects_.begin(), MapObjects_.end()); }

  const MapObject &GetObject(Id<MapObject> Id) const noexcept {
    return MapObjects_.GetObjectById(Id);
  }

  MapObject &GetObject(Id<MapObject> Id) noexcept { return MapObjects_.GetObjectById(Id); }

  std::span<const Id<MapObject>> GetObjectsOnLayer(Dim Layer) const noexcept {
    return ObjectsByLayer_[Layer];
  }

  std::span<const CapitalComponent> GetCapitals() const noexcept {
    return {&*Capitals_.begin(), Capitals_.size()};
  }

  std::span<const Id<MapObject>> GetTowns() const noexcept {
    return {&*Towns_.begin(), Towns_.size()};
  }

  Size GetNumCapitals() const noexcept { return Capitals_.size(); }

  const MapObject &GetCapital(Id<CapitalComponent> CapitalId) const noexcept {
    return GetObject(Capitals_[CapitalId].ObjectId);
  }

  Id<CapitalComponent> AddCapital(CapitalComponent Capital) noexcept {
    Capitals_.push_back(Capital);
    return Capitals_.size() - 1;
  }

private:
  Size Width_;
  Size Height_;
  Size Layers_;
  std::vector<Tile> Tiles_;

  Utils::Registry<MapObject> MapObjects_;
  SmallVector<CapitalComponent, 8> Capitals_;
  std::vector<std::vector<Id<MapObject>>> ObjectsByLayer_;

  // SmallVector<Id<MapObject>, 8> Capitals_;
  SmallVector<Id<MapObject>, 32> Towns_;
};

class Path {
public:
  using Storage = std::vector<Coord3D>;

  Path() noexcept = default;

  const Storage &GetPoints() const noexcept { return Points_; }
  void AddPoint(Coord3D Pt) noexcept { Points_.push_back(Pt); }

private:
  Storage Points_;
  Size Position_ = 0;
};

} // namespace NotAGame

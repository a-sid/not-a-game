#pragma once

#include "entities/common.h"
#include "entities/components.h"
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

enum class TerrainKind { Ground, Water, Relief };

enum class GroundKind { Plain, Road, Forest };
enum class Passability { Passable, Impassable };

class MapObject;
using MapObjectPtr = std::unique_ptr<MapObject>;

class MapObject : public Named {
public:
  enum Kind { Capital, Town, Grave, Rod, Shop, MagicShop, TrainingCamp, Resource, Ruins, Other };

  MapObject(Named Name, Kind Kind, Coord3D Pos, Dims2D Size, std::optional<Coord> EntrancePos,
            Passability Passability) noexcept
      : Named{std::move(Name)}, Kind_{Kind}, Pos_{Pos}, Size_{Size}, EntrancePos_{EntrancePos},
        Passability_{Passability} {}

  Kind GetKind() const noexcept { return Kind_; }
  Coord3D GetPosition() const noexcept { return Pos_; }
  Dim GetLayer() const noexcept { return Pos_.Layer; }
  Dim GetX() const noexcept { return Pos_.X; }
  Dim GetY() const noexcept { return Pos_.Y; }

  Dims2D GetSize() const noexcept { return Size_; }
  Size GetWidth() const noexcept { return Size_.Width; }
  Size GetHeight() const noexcept { return Size_.Height; }

  std::optional<Coord> GetEntrancePos() const noexcept { return EntrancePos_; }

  template <typename T> const T &GetAs() const noexcept {
    assert(GetKind() == T::Class);
    return *static_cast<const T *>(this);
  }

  template <typename T> const T *TryGetAs() const noexcept {
    return (GetKind() == T::Class) ? static_cast<const T *>(this) : nullptr;
  }

  Id<LandPropagation> GetLandPropagation() const noexcept { return LandPropagation_; }
  Id<VisibilityRange> GetVisibilityRange() const noexcept { return VisibilityRange_; }

  virtual void Register(const Mod &M, GameplaySystems &Systems) noexcept {}

protected:
  Kind Kind_;
  Id<Animation> AnimationId_;
  Coord3D Pos_;
  Dims2D Size_;
  std::optional<Coord> EntrancePos_;
  Id<LandPropagation> LandPropagation_;
  Id<VisibilityRange> VisibilityRange_;
  Id<ResourceSource> Source_;
  Passability Passability_;
};

class Terrain : public Named {
public:
  Terrain(Named Name, Size BaseCost) noexcept : Named{std::move(Name)}, BaseCost_{BaseCost} {}
  Size GetBaseCost() const noexcept { return BaseCost_; }

private:
  Size BaseCost_;
};

struct BattleResult {
  std::string Loser;
  std::string Winner;
};

using BattleResults = std::vector<BattleResult>;
class Grave : public MapObject {
public:
  static constexpr Kind Class = Kind::Grave;
  Grave(Coord3D Pos, BattleResult Result) noexcept
      : MapObject{Named{"Grave", "Grave title", "Grave description"},
                  Class,
                  Pos,
                  Dims2D{1, 1},
                  std::nullopt,
                  Passability::Passable} {
    AddBattle(std::move(Result));
  }

  void AddBattle(BattleResult Result) { BattleResults_.push_back(std::move(Result)); }

  const BattleResults &GetBattleResults() const { return BattleResults_; }

private:
  BattleResults BattleResults_;
};

class Rod : public MapObject {
public:
  static constexpr Kind Class = Kind::Rod;
  Rod(const Mod &M, Coord3D Pos, Id<Player> PlayerId) noexcept;

  void Register(const Mod &M, GameplaySystems &Systems) noexcept override {
    LandPropagation_ = Systems.LandPropagation.AddComponent(LandPropagationComponent_);
  }

private:
  LandPropagation LandPropagationComponent_;
};

struct TownSettings {
  Coord EntrancePos;
  Dim Width;
  Dim Height;
};

struct CapitalSettings {
  Coord EntrancePos;
  Dim Width;
  Dim Height;
};

class Town : public MapObject {
public:
  static constexpr Kind Class = Kind::Town;

  Town(const Mod &Mod, Named Name, Coord3D Pos, Size Level, PlayerId PlayerId) noexcept;

  PlayerId GetOwner() const noexcept { return Owner_; }
  void SetOwner(Id<Player> PlayerId) noexcept { Owner_ = PlayerId; }
  bool IsNeutral() const noexcept { return Owner_.IsInvalid(); }

  void Register(const Mod &M, GameplaySystems &Systems) noexcept override {
    LandPropagation_ = Systems.LandPropagation.AddComponent(LandPropagationComponent_);
    VisibilityRange_ = Systems.Visibility.AddComponent(VisibilityRangeComponent_);
  }

protected:
  Size Level_;
  LandPropagation LandPropagationComponent_;
  VisibilityRange VisibilityRangeComponent_;
  Id<Squad> Guard_;
  std::optional<Id<Squad>> Squad_;
  Id<Player> Owner_;
};

class Capital : public MapObject {
public:
  static constexpr Kind Class = Kind::Capital;

  Capital(const Mod &Mod, Named Name, Coord3D Pos, PlayerId PlayerId,
          Id<Fraction> FractionId) noexcept;

  PlayerId GetOwner() const noexcept { return Owner_; }

private:
  LandPropagation LandPropagationComponent_;
  VisibilityRange VisibilityRangeComponent_;
  Id<Squad> Guard_;
  std::optional<Id<Squad>> Squad_;
  Id<Player> Owner_;
  Id<Fraction> FractionId_;
};

class Tile {
public:
  explicit Tile(Id<Terrain> Terrain = NullId) noexcept : Terrain_{Terrain} {}
  Id<Terrain> Terrain_;
  Id<Player> Owner_;
  std::optional<Grave> Grave_;
  Id<MapObjectPtr> Object_ = NullId;
  Id<Squad> Squad_ = NullId;
  // By default, all tiles are visible to the neutral player which has index 0.
  uint16_t VisibilityFlags = 1;

  /*
  bool HasGrave() const noexcept { return Grave_.has_value(); }
  Grave &AddGrave(BattleResult Result) noexcept {
    if (!Grave_) {
      Grave_ = Grave{std::move(Result)};
    } else {
      Grave_->AddBattle(Result);
    }
    return *Grave_;
  }*/
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

  Status AddObject(Dim Layer, Dim X, Dim Y, std::string Name, MapObjectPtr Object) noexcept {
    assert(Object->GetHeight() && Object->GetWidth());
    auto MaxX = X + Object->GetWidth();
    auto MaxY = Y + Object->GetHeight();

    if (MaxX > Width_ || MaxY > Height_) {
      return Status::Error(ErrorCode::MapError, "Coordinates out of bounds");
    }

    if (!CanPlaceObject(Layer, X, Y, *Object)) {
      return Status::Error(ErrorCode::MapError, "Object overlap is not allowed");
    }

    const bool IsCapital = Object->GetKind() == MapObject::Kind::Capital;
    auto Id = MapObjects_.AddObject(std::move(Name), std::move(Object));
    ObjectsByLayer_[Layer].push_back(Id);
    for (Dim x = X; x < MaxX; ++x) {
      for (Dim y = Y; y < MaxY; ++y) {
        GetTile(Layer, x, y).Object_ = Id;
      }
    }

    if (IsCapital) {
      Capitals_.push_back(Id);
    }
    return Status::Success();
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

  const MapObject &GetObject(Id<MapObjectPtr> Id) const noexcept {
    return *MapObjects_.GetObjectById(Id);
  }

  std::span<const Id<MapObjectPtr>> GetObjectsOnLayer(Dim Layer) const noexcept {
    return ObjectsByLayer_[Layer];
  }

  std::span<const Id<MapObjectPtr>> GetCapitals() const noexcept {
    return {&*Capitals_.begin(), Capitals_.size()};
  }

  std::span<const Id<MapObjectPtr>> GetTowns() const noexcept {
    return {&*Towns_.begin(), Towns_.size()};
  }

  Size GetNumCapitals() const noexcept { return Capitals_.size(); }

  const Capital &GetCapital(Id<MapObjectPtr> ObjectId) const noexcept {
    return GetObject(ObjectId).GetAs<Capital>();
  }

private:
  Size Width_;
  Size Height_;
  Size Layers_;
  std::vector<Tile> Tiles_;

  Utils::Registry<MapObjectPtr> MapObjects_;
  std::vector<std::vector<Id<MapObjectPtr>>> ObjectsByLayer_;

  SmallVector<Id<MapObjectPtr>, 8> Capitals_;
  SmallVector<Id<MapObjectPtr>, 32> Towns_;
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

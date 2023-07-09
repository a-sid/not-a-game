#pragma once

#include "entities/common.h"
#include "game/settings.h"
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
class Player;
class Squad;

struct Point {
  Coord X;
  Coord Y;
};

enum class TerrainKind { Ground, Water, Relief };

enum class GroundKind { Plain, Road, Forest };

class MapObject;
using MapObjectPtr = std::unique_ptr<MapObject>;

class MapObject : public Named {
public:
  enum Kind { Capital, Town, Shop, MagicShop, TrainingCamp, Resource, Ruins, Other };

  MapObject(Named Name, Kind Kind, Dim Layer, Dim X, Dim Y, Size Width, Size Height,
            std::optional<Coord> EntrancePos) noexcept
      : Named{std::move(Name)}, Kind_{Kind}, Layer_{Layer}, X_{X}, Y_{Y}, Width_{Width},
        Height_{Height}, EntrancePos_{EntrancePos} {}

  Kind GetKind() const noexcept { return Kind_; }
  Dim GetLayer() const noexcept { return Layer_; }
  Dim GetX() const noexcept { return X_; }
  Dim GetY() const noexcept { return Y_; }
  Size GetWidth() const noexcept { return Width_; }
  Size GetHeight() const noexcept { return Height_; }
  std::optional<Coord> GetEntrancePos() noexcept { return EntrancePos_; }

protected:
  Kind Kind_;
  Id<Animation> AnimationId_;
  Dim Layer_;
  Dim X_;
  Dim Y_;
  Size Width_;
  Size Height_;
  std::optional<Coord> EntrancePos_;
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
class Grave {
public:
  explicit Grave(BattleResult Result) { AddBattle(std::move(Result)); }

  void AddBattle(BattleResult Result) { BattleResults_.push_back(std::move(Result)); }

  const BattleResults &GetBattleResults() const { return BattleResults_; }

private:
  BattleResults BattleResults_;
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
  Town(const TownSettings &Settings, Named Name, Dim Layer, Dim X, Dim Y, Size Level) noexcept;
  Town(Named Name, Dim Layer, Dim X, Dim Y, Dim Width, Dim Height, Coord EntrancePos,
       Size Level) noexcept;

  Id<Player> GetOwner() const noexcept { return Owner_; }
  void SetOwner(Id<Player> PlayerId) noexcept { Owner_ = PlayerId; }
  bool IsNeutral() const noexcept { return Owner_.IsInvalid(); }

private:
  std::string Name_;
  Size Level_;
  Id<Squad> Guard_;
  std::optional<Id<Squad>> Squad_;
  Id<Player> Owner_;
};

class Capital : public Town {
public:
  Capital(const CapitalSettings &Settings, Named Name, Dim Layer, Dim X, Dim Y)
      : Town{std::move(Name),      Layer,   X, Y, Settings.Width, Settings.Height,
             Settings.EntrancePos, MAX_SIZE} {
    Kind_ = Kind::Capital;
  }

private:
};

class Tile {
public:
  explicit Tile(Id<Terrain> Terrain = NullId) noexcept : Terrain_{Terrain} {}
  Id<Terrain> Terrain_;
  std::optional<Id<Player>> Owner_;
  std::optional<Grave> Grave_;
  Id<MapObjectPtr> Object_ = NullId;
  Id<Squad> Squad_ = NullId;

  bool HasGrave() const noexcept { return Grave_.has_value(); }
  Grave &AddGrave(BattleResult Result) noexcept {
    if (!Grave_) {
      Grave_ = Grave{std::move(Result)};
    } else {
      Grave_->AddBattle(Result);
    }
    return *Grave_;
  }
};

class GlobalMap {
public:
  GlobalMap(/*const Utils::Registry<MapObject> &MapObjects,*/ Size Layers, Size Width,
            Size Height) noexcept
      : Width_{Width}, Height_{Height}, Layers_{Layers} {
    Tiles_.resize(Width * Height * Layers);
    ObjectsByLayer_.resize(Layers);
  }

  Size GetWidth() const noexcept { return Width_; }
  Size GetHeight() const noexcept { return Height_; }
  Size GetNumLayers() const noexcept { return Layers_; }

  Status AddObject(Dim Layer, Dim X, Dim Y, std::string Name, MapObjectPtr Object) noexcept {
    assert(Object->GetHeight() && Object->GetWidth());
    auto MaxX = X + Object->GetHeight();
    auto MaxY = Y + Object->GetWidth();

    if (MaxX > Width_ || MaxY > Height_) {
      return Status::Error(ErrorCode::MapError, "Coordinates out of bounds");
    }

    if (!CanPlaceObject(Layer, X, Y, *Object)) {
      return Status::Error(ErrorCode::MapError, "Object overlap is not allowed");
    }
    auto Id = MapObjects_.AddObject(std::move(Name), std::move(Object));
    ObjectsByLayer_[Layer].push_back(Id);
    for (Dim x = X; x < MaxX; ++x) {
      for (Dim y = Y; y < MaxY; ++y) {
        GetTile(Layer, x, y).Object_ = Id;
      }
    }
    return Status::Success();
  }

  bool CanPlaceObject(Dim Layer, Dim X, Dim Y, const MapObject &Object) const noexcept {
    auto MaxX = X + Object.GetHeight();
    auto MaxY = Y + Object.GetWidth();

    for (Dim x = X; x < MaxX; ++x) {
      for (Dim y = Y; y < MaxY; ++y) {
        const auto &Tile = GetTile(Layer, x, y);
        if (Tile.Object_.IsValid() || Tile.HasGrave()) {
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

  const MapObject &GetObject(Id<MapObjectPtr> Id) const noexcept {
    return *MapObjects_.GetObjectById(Id);
  }

  std::span<const Id<MapObjectPtr>> GetObjectsOnLayer(Dim Layer) const noexcept {
    return ObjectsByLayer_[Layer];
  }

private:
  Size Width_;
  Size Height_;
  Size Layers_;
  std::vector<Tile> Tiles_;

  Utils::Registry<MapObjectPtr> MapObjects_;
  std::vector<std::vector<Id<MapObjectPtr>>> ObjectsByLayer_;
  Utils::Registry<Town> Towns_;
};

class Path {
public:
  using Storage = std::vector<Point>;

  Path() noexcept = default;

  const Storage &GetPoints() const noexcept { return Points_; }
  void AddPoint(Point Pt) noexcept { Points_.push_back(Pt); }

private:
  Storage Points_;
  Size Position_ = 0;
};

} // namespace NotAGame

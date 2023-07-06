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

  MapObject(Named Name, Kind Kind, std::optional<Coord> EntrancePos, Size Width,
            Size Height) noexcept
      : Named{std::move(Name)}, Kind_{Kind}, Width_{Width}, Height_{Height}, EntrancePos_{
                                                                                 EntrancePos} {}

  Size GetWidth() const noexcept { return Width_; }
  Size GetHeight() const noexcept { return Height_; }
  std::optional<Coord> GetEntrancePos() noexcept { return EntrancePos_; }

private:
  Kind Kind_;
  Id<Animation> AnimationId_;
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

class Town : public MapObject {
public:
  Town(const TownSettings &Settings, Named Name, Size Level) noexcept;
  Town(Named Name, Coord EntrancePos, Dim Width, Dim Height, Size Level) noexcept;

private:
  std::string Name_;
  Size Level_;
  Id<Squad> Guard_;
  std::optional<Id<Squad>> Squad_;
  Id<Player> Owner_;
};

class Capital : public Town {
public:
  Capital(const CapitalSettings &Settings, Named Name)
      : Town{std::move(Name), Settings.EntrancePos, Settings.Width, Settings.Height, MAX_SIZE} {}

private:
};

class Tile {
public:
  explicit Tile(Id<Terrain> Terrain = NullId) noexcept : Terrain_{Terrain} {}
  Id<Terrain> Terrain_;
  std::optional<Id<Player>> Owner_;
  std::optional<Grave> Grave_;
  Id<MapObjectPtr> Object_ = NullId;
  Id<Squad> Squad_;

  bool HasGrave() const noexcept { return Grave_.has_value(); }
  Grave &AddGrave(BattleResult Result) noexcept {
    if (!Grave_) {
      Grave_ = Grave{std::move(Result)};
    } else {
      Grave_->AddBattle(Result);
    }
    return *Grave_;
  }

  std::string GetDescription() const noexcept;
};

class GlobalMap {
public:
  GlobalMap(/*const Utils::Registry<MapObject> &MapObjects,*/ Size Width, Size Height) noexcept
      : Width_{Width}, Height_{Height} {
    Tiles_.resize(Width * Height);
  }

  Size GetWidth() const noexcept { return Width_; }
  Size GetHeight() const noexcept { return Height_; }

  Status AddObject(Dim X, Dim Y, std::string Name, MapObjectPtr Object) noexcept {
    assert(Object->GetHeight() && Object->GetWidth());
    auto MaxX = X + Object->GetHeight();
    auto MaxY = Y + Object->GetWidth();

    if (MaxX > Width_ || MaxY > Height_) {
      return Status::Error(ErrorCode::MapError, "Coordinates out of bounds");
    }

    if (!CanPlaceObject(X, Y, *Object)) {
      return Status::Error(ErrorCode::MapError, "Object overlap is not allowed");
    }
    auto Id = MapObjects_.AddObject(std::move(Name), std::move(Object));
    for (Dim x = X; x < MaxX; ++x) {
      for (Dim y = Y; y < MaxY; ++y) {
        GetTile(x, y).Object_ = Id;
      }
    }
    return Status::Success();
  }

  bool CanPlaceObject(Dim X, Dim Y, const MapObject &Object) const noexcept {
    auto MaxX = X + Object.GetHeight();
    auto MaxY = Y + Object.GetWidth();

    for (Dim x = X; x < MaxX; ++x) {
      for (Dim y = Y; y < MaxY; ++y) {
        const auto &Tile = GetTile(x, y);
        if (Tile.Object_ || Tile.HasGrave()) {
          return false;
        }
      }
    }
    return true;
  }

  Tile &GetTile(Dim X, Dim Y) noexcept { return Tiles_[X + Y * Width_]; }
  const Tile &GetTile(Dim X, Dim Y) const noexcept { return Tiles_[X + Y * Width_]; }

private:
  Size Width_;
  Size Height_;
  std::vector<Tile> Tiles_;

  Utils::Registry<MapObjectPtr> MapObjects_;
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

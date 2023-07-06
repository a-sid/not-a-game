#pragma once

#include "entities/effect.h"
#include "util/id.h"
#include "util/registry.h"
#include "util/types.h"

#include <cstdint>
#include <deque>
#include <string>

namespace NotAGame {

class Icon;

class Unit;
class Item;

struct AttackSource {
  std::string Description;
  uint64_t Tags;
};

class AttackAction {
  virtual void Apply(const Unit &Attacker, Coord Coord) const noexcept = 0;
  virtual bool CanApply(const Unit &Target) const noexcept = 0;
  virtual const std::string &GetDescription() const noexcept = 0;
};

class Attack {
  Id<AttackSource> AttackSourceId;
  int16_t Value_;
  int16_t Modifier_;

private:
};

using Bitset = uint64_t;
constexpr Size kMaxBitsetId = sizeof(Bitset) * __CHAR_BIT__;

enum class UnitWidth : Dim { NORMAL = 1, BIG = 2 };
enum class UnitHeight : Dim { NORMAL = 1, BIG = 2 };

template <typename T> Dim ToDim(T Val) noexcept { return static_cast<Dim>(Val); }

class Perk;
class Squad;

class LeaderDescriptor {
  Size MaxLeadership;
  Size MaxSteps;

  std::deque<Id<Perk>> Perks;
};

struct TraitGrowth {
  Size MinLevel = 1;
  Size MaxLevel = MAX_SIZE;

  Size Health = 0;
  Size Damage = 0;
  Size Armor = 0;
  Size ExpForKill = 0;
  Size Steps = 0;
};

class UnitTrait {
public:
  virtual void RecomputeEffects(const SmallVector<Effect, 16> &Effects) noexcept = 0;
};

template <typename T> class ValueTrait {
public:
  ValueTrait(T BaseValue) noexcept : BaseValue_{BaseValue}, Value_{BaseValue} {}
  virtual void RecomputeEffects(const SmallVector<Effect, 16> &Effects) noexcept;

  T GetBaseValue() const noexcept { return BaseValue_; }
  T GetValue() const noexcept { return Value_; }

protected:
  T BaseValue_;
  T Value_;
};

template <typename T> class CappedTrait : public ValueTrait<T> {
public:
  CappedTrait(T BaseValue, T Cap) noexcept : ValueTrait<T>{BaseValue}, Cap_{Cap} {}
  virtual void RecomputeEffects(const SmallVector<Effect, 16> &Effects) noexcept {
    ValueTrait<T>::RecomputeEffects(Effects);
    this->Value_ = std::min(this->Value_, Cap_);
  }

private:
  T Cap_;
};

using SizeTrait = ValueTrait<Size>;

class Trait : public ValueTrait<Bitset> {
public:
  virtual void RecomputeEffects(const SmallVector<Effect, 16> &Effects) noexcept;
};

struct UnitDescriptor {
  Id<Icon> IconId;
  std::string Description;
  Size Level;

  Size MaxHealth;
  Size MaxExperience;

  Size Damage;
  Size Defence;

  Size ExpForKill;

  Bitset Immunes;
  Bitset Wards;

  UnitWidth Width;
  UnitHeight Height;

  Id<LeaderDescriptor> LeaderDescriptorId;
};

class LeaderData {
  std::string Name_;

  Size Leadership_;
  Size MaxLeadership_;

  Size Steps_;
  Size MaxSteps_;

  std::deque<Id<Item>> Items_;
};

class Unit {
public:
  Coord GetPosition() const noexcept { return Position_; }
  void SetPosition(Coord Position) noexcept { Position_ = Position; }

  bool IsAlive() const noexcept { return Health_ != 0; }

  bool IsLeader() const noexcept { return Leadership_ != 0; }

  UnitWidth GetWidth() const noexcept { return Registry_.GetObjectById(Descriptor_).Width; }
  UnitHeight GetHeight() const noexcept { return Registry_.GetObjectById(Descriptor_).Height; }

private:
  Utils::Registry<UnitDescriptor> Registry_;
  Id<UnitDescriptor> Descriptor_;
  Id<LeaderData> LeaderData_;

  Size Level_;

  Size Health_;
  Size MaxHealth_;

  Size Experience_;
  Size MaxExperience_;

  Size Damage_;

  Size Leadership_;

  Id<Squad> Squad_;
  Coord Position_;

  std::vector<Id<Effect>> Effects_;
};

} // namespace NotAGame

#pragma once

#include "entities/common.h"
#include "entities/effect.h"
#include "entities/resource.h"
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
template <typename T> Dim ToDim(T Val) noexcept { return static_cast<Dim>(Val); }

class Perk;
class Squad;

struct LeaderDescriptor {
  Size MaxLeadership;
  Size MaxSteps;
  Size ViewRange;

  SmallVector<Id<Perk>, 16> Perks;
};

struct TraitGrowth {
  Size MinLevel = 1;
  Size MaxLevel = MAX_SIZE;

  Size Health = 0;
  Size Damage = 0;
  Size Armor = 0;
  Size Speed = 0;
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
  T GetEffectiveValue() const noexcept { return EffectiveValue_; }
  T GetValue() const noexcept { return Value_; }

protected:
  T BaseValue_;
  T EffectiveValue_;
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

struct UnitDescriptor : public Named {
  UnitDescriptor(Named N, const Utils::Registry<Resource> &ResourceRegistry) noexcept
      : Named{std::move(N)}, HireCost{ResourceRegistry}, ResurrectCost{ResourceRegistry},
        HealPerHPCost{ResourceRegistry} {}

  Id<Icon> IconId;

  Size MaxHealth = 0;
  Size MaxExperience = 0;

  Size Damage = 0;
  Size Armor = 0;

  Size Speed = 0;

  Size ExpForKill = 0;

  Bitset Immunes;
  Bitset Wards;

  uint8_t Width = 1;
  uint8_t Height = 1;

  Id<UnitDescriptor> PreviousForm;

  Id<LeaderDescriptor> LeaderDescriptorId;

  Size HealthGrowth = 0;
  Size DamageGrowth = 0;
  Size ExpForKillGrowth = 0;

  Resources HireCost;
  Resources ResurrectCost;
  Resources HealPerHPCost;
};

class Inventory {

  std::unordered_map<Id<Item>, Size> Items_;
};

class LeaderData {
  std::string Name_;

  SizeTrait Leadership_;
  SizeTrait Steps_;
  SizeTrait ViewRange_;

  Inventory Items_;
};

class Unit {
public:
  Coord GetPosition() const noexcept { return Position_; }
  void SetPosition(Coord Position) noexcept { Position_ = Position; }

  bool IsAlive() const noexcept { return Health_.GetValue() != 0; }

  bool IsLeader() const noexcept { return LeaderData_.IsValid(); }

  uint8_t GetWidth() const noexcept { return Registry_.GetObjectById(Descriptor_).Width; }
  uint8_t GetHeight() const noexcept { return Registry_.GetObjectById(Descriptor_).Height; }

  Size GetLevel() const noexcept { return Level_; }
  const SizeTrait &GetHealth() const noexcept { return Health_; }

private:
  Utils::Registry<UnitDescriptor> &Registry_;
  Id<UnitDescriptor> Descriptor_;
  Id<LeaderData> LeaderData_;

  Size Level_;

  SizeTrait Health_;
  SizeTrait Experience_;
  SizeTrait Speed_;

  SizeTrait Damage_;

  Id<Squad> Squad_;
  Coord Position_;

  std::vector<Id<Effect>> Effects_;

  bool IsMovable_ = true;
};

} // namespace NotAGame

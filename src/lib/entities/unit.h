#pragma once

#include "entities/common.h"
//#include "entities/components.h"
#include "entities/effect.h"
#include "entities/inventory.h"
#include "entities/resource.h"
#include "ui/icon.h"
#include "util/id.h"
#include "util/registry.h"
#include "util/types.h"

#include <cstdint>
#include <deque>
#include <span>
#include <string>

namespace NotAGame {

class Unit;
class Squad;
class GameplaySystems;

enum class ActionSquad { Enemy, Friendly };

using ReachableUnits = SmallVector<Unit *, 16>;

struct ActionRange {
public:
  virtual ReachableUnits ComputeReachableUnits(GameplaySystems &Systems, Squad &EnemySquad,
                                               Squad &FriendSquad,
                                               const Unit &U) const noexcept = 0;
};

class SquadRange : public ActionRange {
public:
  SquadRange(ActionSquad Target) : Target{Target} {}
  ReachableUnits ComputeReachableUnits(GameplaySystems &Systems, Squad &EnemySquad,
                                       Squad &FriendSquad, const Unit &U) const noexcept;

protected:
  virtual ReachableUnits ComputeReachableUnitsInSquad(GameplaySystems &Systems, Squad &S,
                                                      const Unit &U) const noexcept = 0;

private:
  ActionSquad Target;
};

class NearestUnitRange final : public ActionRange {
public:
  explicit NearestUnitRange(Size GridWidth) : GridWidth_{GridWidth} {}
  ReachableUnits ComputeReachableUnits(GameplaySystems &Systems, Squad &EnemySquad,
                                       Squad &FriendSquad, const Unit &U) const noexcept override;

private:
  Size GridWidth_;
};

class AnyUnitRange final : public SquadRange {
public:
  AnyUnitRange(ActionSquad Target) : SquadRange{Target} {}

protected:
  ReachableUnits ComputeReachableUnitsInSquad(GameplaySystems &Systems, Squad &S,
                                              const Unit &U) const noexcept override;
};

class AllUnitRange final : public SquadRange {
public:
  AllUnitRange(ActionSquad Target) : SquadRange{Target} {}

protected:
  ReachableUnits ComputeReachableUnitsInSquad(GameplaySystems &Systems, Squad &S,
                                              const Unit &U) const noexcept override;
};

struct ActionSource : public Named {
  ActionSource(Named Name) : Named{std::move(Name)} {}
  using Named::Named;
  Id<ActionSource> ComponentId;
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
  ValueTrait() noexcept = default;
  ValueTrait(const ValueTrait &RHS) noexcept = default;
  ValueTrait &operator=(const ValueTrait &RHS) noexcept = default;

  ValueTrait(T BaseValue) noexcept
      : BaseValue_{BaseValue}, Value_{BaseValue}, EffectiveValue_{BaseValue} {}
  virtual void RecomputeEffects(const SmallVector<Effect, 16> &Effects) noexcept {}

  T GetBaseValue() const noexcept { return BaseValue_; }
  T GetEffectiveValue() const noexcept { return EffectiveValue_; }
  T GetValue() const noexcept { return Value_; }

  void SetValue(T Value) noexcept { Value_ = Value; }

protected:
  T BaseValue_;
  T EffectiveValue_;
  T Value_;
};

template <typename T> class CappedTrait : public ValueTrait<T> {
public:
  CappedTrait() noexcept = default;
  CappedTrait(const CappedTrait &RHS) noexcept = default;
  CappedTrait &operator=(const CappedTrait &RHS) noexcept = default;

  CappedTrait(T BaseValue, T Cap) noexcept : ValueTrait<T>{BaseValue}, Cap_{Cap} {}
  virtual void RecomputeEffects(const SmallVector<Effect, 16> &Effects) noexcept {
    ValueTrait<T>::RecomputeEffects(Effects);
    this->Value_ = std::min(this->Value_, Cap_);
  }

private:
  T Cap_;
};

using SizeTrait = ValueTrait<Size>;

class BitsetTrait : public ValueTrait<Bitset> {
public:
  virtual void RecomputeEffects(std::span<Effect> Effects) noexcept;
};

struct EffectAction {
  EffectAction(Id<ActionSource> ActionSource, std::string Description) noexcept
      : Source{ActionSource}, Description{std::move(Description)} {}

  virtual ~EffectAction() noexcept = default;

  virtual void Apply(Unit &Attacker, Unit &Target) noexcept = 0;
  virtual std::unique_ptr<EffectAction> Clone() const noexcept = 0;

  Id<ActionSource> Source;
  std::string Description;
};

struct SubAction {
  CappedTrait<uint8_t> Accuracy;
  std::unique_ptr<EffectAction> Effect;

  ~SubAction() noexcept = default;
  SubAction() noexcept = default;
  SubAction(const SubAction &RHS) noexcept : Accuracy{RHS.Accuracy}, Effect{RHS.Effect->Clone()} {}
  SubAction(SubAction &&) noexcept = default;

  SubAction &operator=(const SubAction &RHS) noexcept {
    Accuracy = RHS.Accuracy;
    Effect = RHS.Effect->Clone();
    return *this;
  }
  SubAction &operator=(SubAction &&) noexcept = default;
};

struct UnitAction {
  Size Index;
  const ActionRange *Range;
  SmallVector<SubAction, 2> SubActions;
};
/*
class AttackAction {
  virtual void Apply(const Unit &Attacker, Coord Coord) const noexcept = 0;
  virtual bool CanApply(const Unit &Target) const noexcept = 0;
  virtual const std::string &GetDescription() const noexcept = 0;
};
*/
struct Skill {};

struct LeaderData {
  Id<LeaderData> ComponentId;

  std::string Name;

  SizeTrait Leadership;
  SizeTrait Steps;
  SizeTrait ViewRange;

  Inventory Items;
  SmallVector<Skill, 8> Skills;
};

struct UnitDescriptor : public Named {
  UnitDescriptor(Named N, const ResourceRegistry &ResourceRegistry) noexcept
      : Named{std::move(N)}, HireCost{ResourceRegistry}, ResurrectCost{ResourceRegistry},
        HealPerHPCost{ResourceRegistry} {}

  Id<Icon> GridIconId;
  Id<Icon> InfoIconId;

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
// class Unit {
// public:
//   Coord GetPosition() const noexcept { return Position_; }
//   void SetPosition(Coord Position) noexcept { Position_ = Position; }

//  bool IsAlive() const noexcept { return Health_.GetValue() != 0; }

//  bool IsLeader() const noexcept { return LeaderData_.IsValid(); }

//  uint8_t GetWidth() const noexcept { return Registry_.GetObjectById(Descriptor_).Width; }
//  uint8_t GetHeight() const noexcept { return Registry_.GetObjectById(Descriptor_).Height; }

//  Size GetLevel() const noexcept { return Level_; }
//  const SizeTrait &GetHealth() const noexcept { return Health_; }

// private:
//   Utils::Registry<UnitDescriptor> &Registry_;
//   Id<UnitDescriptor> Descriptor_;
//   Id<LeaderData> LeaderData_;

//  Size Level_;

//  SizeTrait Health_;
//  SizeTrait Experience_;
//  SizeTrait Speed_;

//  SizeTrait Damage_;

//  Id<Squad> Squad_;
//  Coord Position_;

//  std::vector<Id<Effect>> Effects;

//  bool IsMovable_ = true;
//};

class Unit : public Named {
public:
  Unit(Named N, const ResourceRegistry &ResourceRegistry) noexcept
      : Named{std::move(N)}, HireCost{ResourceRegistry}, ResurrectCost{ResourceRegistry},
        HealPerHPCost{ResourceRegistry} {}

  // Unit(Unit &&) noexcept = default;

  Id<Unit> ComponentId;

  IconSet GridIcons;
  Id<Icon> InfoIconId;

  Size ExpForKill = 0;

  Bitset Immunes;
  Bitset Wards;

  uint8_t Width = 1;
  uint8_t Height = 1;

  Id<Unit> PreviousForm;

  Id<LeaderData> LeaderDataId; // Valid iff a unit is a leader.

  Size HealthGrowth = 0;
  Size DamageGrowth = 0;
  Size ExpForKillGrowth = 0;

  Resources HireCost;
  Resources ResurrectCost;
  Resources HealPerHPCost;

  Size Level;

  SizeTrait Health;
  SizeTrait Experience;
  SizeTrait Speed;

  std::vector<UnitAction> BattleActions;
  CappedTrait<Size> Armor;

  Id<Squad> SquadId;
  Coord GridPosition;

  std::vector<Id<Effect>> Effects;

  bool IsMovable_ = true;

  bool IsDead() const noexcept { return Health.GetValue() == 0; }
  bool IsAlive() const noexcept { return !IsDead(); }

  bool IsLeader() const noexcept { return LeaderDataId.IsValid(); }

private:
  // Utils::Registry<UnitDescriptor> &Registry_;
};

} // namespace NotAGame

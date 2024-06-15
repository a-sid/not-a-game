#pragma once

#include "entities/unit.h"

namespace NotAGame {

struct DirectDamageAction : public EffectAction {
  DirectDamageAction(Size Amount, Id<ActionSource> ActionSource, std::string Description) noexcept
      : EffectAction{ActionSource, std::move(Description)}, Amount{Amount} {}

  void Apply(Unit &Attacker, Unit &Target) noexcept override {
    Size Damage = Amount * (100 - Target.Armor.GetValue()) / 100;
    Size EffectiveDamage = std::min(Damage, Target.Health.GetValue());
    Target.Health.SetValue(Target.Health.GetValue() - EffectiveDamage);
  }

  std::unique_ptr<EffectAction> Clone() const noexcept override {
    return std::make_unique<DirectDamageAction>(*this);
  }

  Size Amount;
};

struct CriticalDamageAction : public EffectAction {
  CriticalDamageAction(Size Amount, Id<ActionSource> ActionSource, std::string Description) noexcept
      : EffectAction{ActionSource, std::move(Description)}, Amount{Amount} {}

  void Apply(Unit &Attacker, Unit &Target) noexcept override {
    Size EffectiveDamage = std::min(Amount, Target.Health.GetValue());
    Target.Health.SetValue(Target.Health.GetValue() - EffectiveDamage);
  }

  std::unique_ptr<EffectAction> Clone() const noexcept override {
    return std::make_unique<CriticalDamageAction>(*this);
  }

  Size Amount;
};

} // namespace NotAGame

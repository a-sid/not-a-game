#pragma once

#include "entities/building.h"
#include "util/types.h"

#include <deque>

namespace NotAGame {

enum class EffectKind {
  Buff,
  Debuff,
  Potion,
};

enum class EffectTarget {
  BaseValue,
  PercentValue,
  FinalValue,
  SetBit,
  SetValue,
};

class SubEffect {
public:
private:
  EffectKind EffectKind_;
  EffectTarget EffectTarget_;
  bool IsBattleLocal_;

  Size TurnsRemaining_;
};

class Effect {
  SmallVector<SubEffect, 4> SubEffects_;
};

} // namespace NotAGame

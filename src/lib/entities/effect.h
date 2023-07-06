#pragma once

#include "entities/building.h"
#include "entities/unit.h"
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

class Effect {
public:
private:
  EffectKind EffectKind_;
  EffectTarget EffectTarget_;
  bool IsBattleLocal_;

  Size TurnsRemaining_;
};
} // namespace NotAGame

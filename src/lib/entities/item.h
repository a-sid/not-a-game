#pragma once

#include "util/id.h"

namespace NotAGame {

class Icon;
class Effect;

class Item {
public:
  enum class Kind { Artifact, Banner, Book, Boots, Orb, Potion, Scroll, Staff, Other };

  Item(Kind Kind, Id<Effect> Effect, Id<Icon> Icon) noexcept
      : Kind_{Kind}, Effect_{Effect}, Icon_{Icon} {}

  Kind GetKind() const noexcept { return Kind_; }
  Id<Icon> GetIcon() const noexcept { return Icon_; }

private:
  Kind Kind_;
  Id<Effect> Effect_;
  Id<Icon> Icon_;
};

} // namespace NotAGame

#pragma once

#include "entities/building.h"
#include "entities/unit.h"
#include "util/types.h"

#include <deque>

namespace NotAGame {

class Spell {
  std::string Name_;
  std::string Description_;
  Size Level_;
  Effect Effect_;
};

} // namespace NotAGame

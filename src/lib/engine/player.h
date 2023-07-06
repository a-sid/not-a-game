#pragma once

#include "entities/fraction.h"
#include "entities/global_map.h"
#include "entities/squad.h"
#include "entities/unit.h"
#include "util/types.h"

#include <string>

struct Color {
  uint8_t Hue;
};

namespace NotAGame {

class Player {
public:
private:
  std::string Name_;
  Id<Fraction> FractionId_;
  Id<Lord> Lord_;
  Id<Icon> IconId_;
  Color Color_;
};

} // namespace NotAGame

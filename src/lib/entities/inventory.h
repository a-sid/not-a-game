#pragma once

#include "util/id.h"
#include "util/types.h"

#include <map>

namespace NotAGame {

class Item;

struct Inventory {
  std::map<Id<Item>, Size> Items;
};

} // namespace NotAGame

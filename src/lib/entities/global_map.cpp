#include "global_map.h"

namespace NotAGame {

Town::Town(const TownSettings &Settings, Named Name, Size Level) noexcept
    : Town{std::move(Name), Settings.EntrancePos, Settings.Width, Settings.Height, Level} {}

Town::Town(Named Name, Coord EntrancePos, Dim Width, Dim Height, Size Level) noexcept
    : MapObject{std::move(Name), Kind::Town, EntrancePos, Width, Height}, Level_{Level} {}

} // namespace NotAGame

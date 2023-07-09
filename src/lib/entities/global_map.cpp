#include "global_map.h"

namespace NotAGame {

Town::Town(const TownSettings &Settings, Named Name, Dim Layer, Dim X, Dim Y, Size Level) noexcept
    : Town{std::move(Name),      Layer, X, Y, Settings.Width, Settings.Height,
           Settings.EntrancePos, Level} {}

Town::Town(Named Name, Dim Layer, Dim X, Dim Y, Dim Width, Dim Height, Coord EntrancePos,
           Size Level) noexcept
    : MapObject{std::move(Name), Kind::Town, Layer, X, Y, Width, Height, EntrancePos}, Level_{
                                                                                           Level} {}

} // namespace NotAGame

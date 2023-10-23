#include "global_map.h"
#include "game/mod.h"

namespace NotAGame {

Town::Town(const Mod &Mod, Named Name, Coord3D Pos, Size Level, PlayerId PlayerId) noexcept
    : MapObject{std::move(Name),
                Class,
                Pos,
                Dims2D{Mod.GetTownSettings().Width, Mod.GetTownSettings().Height},
                Mod.GetTownSettings().EntrancePos,
                Passability::Impassable},
      Level_{Level},
      LandPropagationComponent_{.TilesPerTurn =
                                    Mod.GetLandPropagationSettings().TownPropagationByLevel[Level],
                                .Origin = Pos,
                                .OriginSize = {.Width = Mod.GetTownSettings().Width,
                                               .Height = Mod.GetTownSettings().Height}},
      VisibilityRangeComponent_{
          .Player = PlayerId,
          .Origin = Pos,
          .OriginSize{.Width = Mod.GetTownSettings().Width, .Height = Mod.GetTownSettings().Height},
          .Radius = Mod.GetVisibilityRangeSettings().TownVisibilityByLevel[Level]} {}

Capital::Capital(const Mod &Mod, Named Name, Coord3D Pos, PlayerId PlayerId,
                 Id<Fraction> FractionId) noexcept
    : MapObject{std::move(Name),
                Class,
                Pos,
                Dims2D{Mod.GetCapitalSettings().Width, Mod.GetCapitalSettings().Height},
                Mod.GetCapitalSettings().EntrancePos,
                Passability::Impassable},
      LandPropagationComponent_{
          .Player = PlayerId,
          .TilesPerTurn = Mod.GetLandPropagationSettings().CapitalPropagation,
          .Origin = Pos,
          .OriginSize = {Mod.GetCapitalSettings().Width, Mod.GetCapitalSettings().Height}},
      VisibilityRangeComponent_{.Player = PlayerId,
                                .Origin = Pos,
                                .OriginSize{.Width = Mod.GetCapitalSettings().Width,
                                            .Height = Mod.GetCapitalSettings().Height},
                                .Radius = Mod.GetVisibilityRangeSettings().CapitalVisibility} {}

Rod::Rod(const Mod &M, Coord3D Pos, Id<Player> PlayerId) noexcept
    : MapObject{Named{"Rod", "Rod title", "Rod description"},
                Class,
                Pos,
                Dims2D{1, 1},
                std::nullopt,
                Passability::Impassable},
      LandPropagationComponent_{.Player = PlayerId,
                                .TilesPerTurn = M.GetLandPropagationSettings().RodPropagation,
                                .Origin = Pos,
                                .OriginSize = {1, 1}} {}

} // namespace NotAGame

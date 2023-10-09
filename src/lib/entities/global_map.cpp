#include "global_map.h"
#include "game/mod.h"

namespace NotAGame {

Town::Town(const Mod &Mod, GameplaySystems &Systems, Named Name, Dim Layer, Dim X, Dim Y,
           Size Level, PlayerId PlayerId) noexcept
    : MapObject{std::move(Name),
                Class,
                Layer,
                X,
                Y,
                Mod.GetTownSettings().Width,
                Mod.GetTownSettings().Height,
                Mod.GetTownSettings().EntrancePos},
      Level_{Level}, LandPropagation_{Systems.LandPropagation.AddComponent(LandPropagation{
                         .TilesPerTurn =
                             Mod.GetLandPropagationSettings().TownPropagationByLevel[Level],
                         .Origin = {.X = X, .Y = Y},
                         .OriginSize = {.Width = Mod.GetTownSettings().Width,
                                        .Height = Mod.GetTownSettings().Height}})},
      VisibilityRange_{Systems.Visibility.AddComponent(VisibilityRange{
          .Player = PlayerId,
          .Origin = {.X = X, .Y = Y, .Layer = Layer},
          .OriginSize{.Width = Mod.GetTownSettings().Width, .Height = Mod.GetTownSettings().Height},
          .Radius = Mod.GetVisibilityRangeSettings().TownVisibilityByLevel[Level]})} {}

Capital::Capital(const Mod &Mod, GameplaySystems &Systems, Named Name, Dim Layer, Dim X, Dim Y,
                 PlayerId PlayerId, Id<Fraction> FractionId) noexcept
    : MapObject{std::move(Name),
                Class,
                Layer,
                X,
                Y,
                Mod.GetCapitalSettings().Width,
                Mod.GetCapitalSettings().Height,
                Mod.GetCapitalSettings().EntrancePos},
      LandPropagation_{Systems.LandPropagation.AddComponent(LandPropagation{
          .Player = PlayerId,
          .TilesPerTurn = Mod.GetLandPropagationSettings().CapitalPropagation,
          .Origin = {.X = X, .Y = Y},
          .OriginSize = {Mod.GetCapitalSettings().Width, Mod.GetCapitalSettings().Height}})},
      VisibilityRange_{Systems.Visibility.AddComponent(
          VisibilityRange{.Player = PlayerId,
                          .Origin = {.X = X, .Y = Y, .Layer = Layer},
                          .OriginSize{.Width = Mod.GetCapitalSettings().Width,
                                      .Height = Mod.GetCapitalSettings().Height},
                          .Radius = Mod.GetVisibilityRangeSettings().CapitalVisibility})} {}

Rod::Rod(const Mod &M, GameplaySystems &GameplaySystems, Dim Layer, Dim X, Dim Y,
         Id<Player> PlayerId) noexcept
    : MapObject{Named{"Rod", "Rod", "Rod"}, Class, Layer, X, Y, 1, 1, std::nullopt},
      LandPropagation_{GameplaySystems.LandPropagation.AddComponent(
          LandPropagation{.Player = PlayerId,
                          .TilesPerTurn = M.GetLandPropagationSettings().RodPropagation,
                          .Origin = {X, Y},
                          .OriginSize = {1, 1}})} {}

} // namespace NotAGame

#pragma once

#include "entities/components.h"
#include "entities/fraction.h"
#include "entities/global_map.h"
#include "entities/lord.h"
#include "entities/resource.h"
#include "entities/squad.h"
#include "entities/unit.h"
#include "ui/icon.h"

#include "game/settings.h"

#include "util/registry.h"

#include <filesystem>

namespace NotAGame {

struct Language {
  std::string Value;
};

struct BuildingPage : public Named {
  using Named::Named;
  BuildingPage(Named &&Name) noexcept : Named{std::move(Name)} {}
};

class ModLoader;

class Mod : public Named {
public:
  static Mod Load(const std::filesystem::path &Path) noexcept;
  static Mod Load(const std::string &Name) noexcept;

  // TODO: Should we actually turn it into struct?
  const auto &GetRanges() const noexcept { return ActionRanges_; }
  const auto &GetTerrains() const noexcept { return Terrains_; }
  const auto &GetResources() const noexcept { return Resources_; }
  const auto &GetSpells() const noexcept { return Spells_; }
  const auto &GetFractions() const noexcept { return Fractions_; }
  const auto &GetLords() const noexcept { return Lords_; }
  const auto &GetLeaderPresets() const noexcept { return LeaderPresets_; }
  const auto &GetUnitPresets() const noexcept { return UnitPresets_; }
  const auto &GetIcons() const noexcept { return Icons_; }

  const GridSettings &GetGridSettings() const noexcept { return GridSettings_; }
  const InterfaceSettings &GetInterfaceSettings() const noexcept { return InterfaceSettings_; }

  const LandPropagationSettings &GetLandPropagationSettings() const noexcept {
    return LandPropagationSettings_;
  }
  const VisibilityRangeSettings &GetVisibilityRangeSettings() const noexcept {
    return VisibilityRangeSettings_;
  }

private:
  explicit Mod(Named Name) noexcept;

  void InitRangeMechanics() noexcept;

  friend class NotAGame::ModLoader;

  Utils::Registry<std::unique_ptr<ActionRange>, 8> ActionRanges_;

  Utils::Registry<Terrain, 8> Terrains_;
  ResourceRegistry Resources_;

  Utils::Registry<Unit> UnitPresets_;
  Utils::Registry<LeaderData> LeaderPresets_;
  Utils::Registry<ActionSource, 16> ActionSources_;
  Utils::Registry<std::unique_ptr<UnitTrait>> UnitTraits_;
  Utils::Registry<Icon, 256> Icons_;
  //  Utils::Registry<ItemKind> ItemKinds_;
  // Utils::Registry<Item> Items_;
  Utils::Registry<Spell> Spells_;
  // Utils::Registry<MapObjectPreset> MapObjectPresets_;
  Utils::Registry<Fraction, 8> Fractions_;
  Utils::Registry<Lord, 8> Lords_;
  Utils::Registry<BuildingPage, 8> BuildingPages_;

  LandPropagationSettings LandPropagationSettings_;
  VisibilityRangeSettings VisibilityRangeSettings_;
  GridSettings GridSettings_;
  InterfaceSettings InterfaceSettings_;
};

} // namespace NotAGame

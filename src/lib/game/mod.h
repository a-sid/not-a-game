#pragma once

#include "entities/fraction.h"
#include "entities/global_map.h"
#include "entities/lord.h"
#include "entities/resource.h"
#include "entities/squad.h"
#include "entities/unit.h"

#include "game/settings.h"

#include "util/registry.h"

#include <filesystem>

namespace NotAGame {

struct DamageSource {
  std::string Name;
};

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
  const auto &GetTerrains() const noexcept { return Terrains_; }
  const auto &GetResources() const noexcept { return Resources_; }
  const auto &GetSpells() const noexcept { return Spells_; }
  const auto &GetFractions() const noexcept { return Fractions_; }

  const TownSettings &GetTownSettings() const noexcept { return TownSettings_; }
  const CapitalSettings &GetCapitalSettings() const noexcept { return CapitalSettings_; }
  const GridSettings &GetGridSettings() const noexcept { return GridSettings_; }
  const InterfaceSettings &GetInterfaceSettings() const noexcept { return InterfaceSettings_; }

private:
  explicit Mod(Named Name) noexcept;

  friend class NotAGame::ModLoader;

  Utils::Registry<Terrain, 8> Terrains_;
  ResourceRegistry Resources_;
  Utils::Registry<UnitDescriptor> UnitDescriptors_;
  Utils::Registry<LeaderDescriptor> LeaderDescriptors_;
  Utils::Registry<DamageSource, 16> DamageSources_;
  Utils::Registry<std::unique_ptr<UnitTrait>> UnitTraits_;
  //  Utils::Registry<ItemKind> ItemKinds_;
  // Utils::Registry<Item> Items_;
  Utils::Registry<Spell> Spells_;
  // Utils::Registry<MapObjectPreset> MapObjectPresets_;
  Utils::Registry<Fraction, 8> Fractions_;
  Utils::Registry<Lord, 8> Lords_;
  Utils::Registry<BuildingPage, 8> BuildingPages_;

  TownSettings TownSettings_;
  CapitalSettings CapitalSettings_;
  GridSettings GridSettings_;
  InterfaceSettings InterfaceSettings_;
};

} // namespace NotAGame

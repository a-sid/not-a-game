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

class ModLoader;

class Mod : public Named {
public:
  static Mod Load(const std::filesystem::path &Path) noexcept;
  static Mod Load(const std::string &Name) noexcept;

  const auto &GetTerrains() const noexcept { return Terrains_; }
  const auto &GetResources() const noexcept { return Resources_; }
  const TownSettings &GetTownSettings() const noexcept { return TownSettings_; }
  const CapitalSettings &GetCapitalSettings() const noexcept { return CapitalSettings_; }
  const GridSettings &GetGridSettings() const noexcept { return GridSettings_; }
  const InterfaceSettings &GetInterfaceSettings() const noexcept { return InterfaceSettings_; }

private:
  explicit Mod(Named Name) noexcept;

  friend class NotAGame::ModLoader;

  Utils::Registry<Terrain> Terrains_;
  Utils::Registry<Resource> Resources_;
  Utils::Registry<UnitDescriptor> UnitDescriptors_;
  Utils::Registry<LeaderDescriptor> LeaderDescriptors_;
  Utils::Registry<DamageSource> DamageSources_;
  Utils::Registry<std::unique_ptr<UnitTrait>> UnitTraits_;
  //  Utils::Registry<ItemKind> ItemKinds_;
  //  Utils::Registry<Spell> Spells_;
  // Utils::Registry<MapObjectPreset> MapObjectPresets_;
  Utils::Registry<Building> Buildings_;
  Utils::Registry<Fraction> Fractions_;
  Utils::Registry<Lord> Lords_;

  TownSettings TownSettings_;
  CapitalSettings CapitalSettings_;
  GridSettings GridSettings_;
  InterfaceSettings InterfaceSettings_;
};

} // namespace NotAGame

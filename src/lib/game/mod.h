#pragma once

#include "entities/fraction.h"
#include "entities/global_map.h"
#include "entities/lord.h"
#include "entities/resource.h"
#include "entities/unit.h"
#include "util/registry.h"

#include <filesystem>

class Unit;

namespace NotAGame {

struct DamageSource {
  std::string Name;
};

struct Language {
  std::string Value;
};

class Mod : public Named {
public:
  static Mod Load(const std::filesystem::path &Path) noexcept;
  static Mod Load(const std::string &Name) noexcept;

  const auto &GetTerrains() const noexcept { return Terrains_; }

private:
  explicit Mod(Named Name) noexcept;
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
};

} // namespace NotAGame

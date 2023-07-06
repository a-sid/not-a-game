#include "game/mod.h"
#include "util/settings.h"

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include <fstream>

namespace NotAGame {

template <typename Value> std::string GetString(const Value &V) noexcept {
  return std::string{V.GetString(), V.GetStringLength()};
}

template <typename Value> Named LoadNamed(const Value &V) noexcept {
  auto Name = GetString(V["name"]);
  auto Title = GetString(V["title"][Utils::DEFAULT_LANG]);
  auto Description = GetString(V["description"][Utils::DEFAULT_LANG]);
  return {std::move(Name), std::move(Title), std::move(Description)};
}

template <typename Value> Terrain LoadTerrain(const Value &V) noexcept {
  auto &&Named = LoadNamed(V);
  auto BaseCost = V["cost"].GetUint();
  return Terrain{std::move(Named), BaseCost};
}

Mod::Mod(Named Name) noexcept : Named{std::move(Name)} {}

Mod Mod::Load(const std::filesystem::path &Path) noexcept {
  std::ifstream RootFile{Path / "mod.json"};
  rapidjson::IStreamWrapper JsonStream{RootFile};
  rapidjson::Document Doc;
  Doc.ParseStream(JsonStream);
  Named ModName = LoadNamed(Doc);
  Mod M{std::move(ModName)};

  for (const auto &V : Doc["terrains"].GetArray()) {
    auto T = LoadTerrain(V);
    auto Name = T.GetName();
    M.Terrains_.AddObject(std::move(Name), std::move(T));
  }

  return M;
}

Mod Mod::Load(const std::string &Name) noexcept {
  const auto Path = Utils::GetModsDir() / Name;
  return Load(Path);
}

} // namespace NotAGame

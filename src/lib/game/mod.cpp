#include "game/mod.h"
#include "util/settings.h"

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include <fstream>

namespace NotAGame {

namespace {

template <typename Value> std::string GetString(const Value &V) noexcept {
  return std::string{V.GetString(), V.GetStringLength()};
}

template <typename Value> Named LoadNamed(const Value &V) noexcept {
  auto Name = GetString(V["name"]);
  auto Title = GetString(V["title"][Utils::DEFAULT_LANG]);
  auto Description = GetString(V["description"][Utils::DEFAULT_LANG]);
  return {std::move(Name), std::move(Title), std::move(Description)};
}

template <typename Settings, typename Value>
void LoadDim2DSettings(Settings &S, const Value &V) noexcept {
  S.Width = V["width"].GetUint();
  S.Height = V["height"].GetUint();
}

template <typename Value> Terrain LoadTerrain(const Value &V) noexcept {
  auto &&Named = LoadNamed(V);
  auto BaseCost = V["cost"].GetUint();
  return Terrain{std::move(Named), BaseCost};
}

template <typename Settings, typename Value> Settings LoadTownSettings(const Value &V) noexcept {
  Settings S;
  LoadDim2DSettings(S, V);
  const auto &EntrancePos = V["entrance_pos"];
  S.EntrancePos.X = EntrancePos["x"].GetUint();
  S.EntrancePos.Y = EntrancePos["y"].GetUint();
  return S;
}

template <typename Value> GridSettings LoadGridSettings(const Value &V) noexcept {
  GridSettings S;
  LoadDim2DSettings(S, V);
  return S;
}

template <typename Value> InterfaceSettings LoadInterfaceSettings(const Value &V) noexcept {
  InterfaceSettings S;
  S.GridSpacerHeight = V["grid_spacer_width"].GetUint();
  const auto &UnitIconSettings = V["unit_icon"];
  LoadDim2DSettings(S.UnitGridSize, UnitIconSettings["grid"]);
  LoadDim2DSettings(S.UnitInfoSize, UnitIconSettings["info"]);
  return S;
}

} // namespace

Mod::Mod(Named Name) noexcept : Named{std::move(Name)} {}

class ModLoader {
public:
  static Mod Load(const std::filesystem::path &Path) noexcept {
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

    M.TownSettings_ = LoadTownSettings<TownSettings>(Doc["town_settings"]);
    M.CapitalSettings_ = LoadTownSettings<CapitalSettings>(Doc["capital_settings"]);
    M.GridSettings_ = LoadGridSettings(Doc["grid_settings"]);
    M.InterfaceSettings_ = LoadInterfaceSettings(Doc["interface_settings"]);

    LoadResources(M, Path / "resources");
    LoadUnits(M, Path / "units");
    LoadSpells(M, Path / "spells");

    return M;
  }

  static Mod Load(const std::string &Name) noexcept {
    const auto Path = Utils::GetModsDir() / Name;
    return Load(Path);
  }

private:
  template <typename ParserFn>
  static void LoadDirectory(Mod &M, const std::filesystem::path &Path, const std::string &Filename,
                            ParserFn &&Parser) noexcept {
    for (std::filesystem::directory_iterator I{Path}, E; I != E; ++I) {
      assert(I->is_directory());
      std::ifstream DescrFile{I->path() / Filename};
      rapidjson::IStreamWrapper JsonStream{DescrFile};
      rapidjson::Document Doc;
      Doc.ParseStream(JsonStream);
      Parser(M, I->path(), Doc);
    }
  }

  static void LoadUnits(Mod &M, const std::filesystem::path &Path) noexcept {
    LoadDirectory(M, Path, "unit.json", LoadUnitDescriptor);
  }

  template <typename Value>
  static Resources ParseResources(const Mod &M, const Value &Doc) noexcept {
    const auto &Map = Doc.GetObject();
    const auto &Registry = M.GetResources();
    Resources R{Registry};
    for (const auto &Member : Map) {
      const auto Id = Registry.GetId(GetString(Member.name));
      R.SetAmountById(Id, Member.value.GetInt());
    }
    return R;
  }

  static void LoadUnitDescriptor(Mod &M, const std::filesystem::path &Path,
                                 const rapidjson::Document &Doc) noexcept {
    Named Named = LoadNamed(Doc);
    auto Name = Named.GetName();
    UnitDescriptor U{std::move(Named), M.GetResources()};
    U.Width = Doc["width"].GetUint();
    U.Height = Doc["height"].GetUint();

    U.Armor = Doc["armor"].GetUint();
    U.ExpForKill = Doc["exp_for_kill"].GetUint();
    // TODO U.Immunes
    // TODO U.IconId
    U.MaxExperience = Doc["exp"].GetUint();
    U.MaxHealth = Doc["health"].GetUint();
    U.Speed = Doc["speed"].GetUint();
    // TODO U.Wards

    if (Doc.HasMember("prev_form")) {
      U.PreviousForm = M.UnitDescriptors_.GetId(GetString(Doc["prev_form"]));
    }

    const auto &Growth = Doc["growth"][0]; // TODO
    U.DamageGrowth = Growth["damage"].GetUint();
    U.HealthGrowth = Growth["health"].GetUint();
    U.ExpForKillGrowth = Growth["exp_for_kill"].GetUint();

    const auto &Costs = Doc["costs"];
    U.HireCost = ParseResources(M, Costs["hire"]);
    U.ResurrectCost = ParseResources(M, Costs["resurrect"]);
    U.HealPerHPCost = ParseResources(M, Costs["heal"]);

    if (Doc.HasMember("leader")) {
      const auto &LeaderData = Doc["leader"];
      LeaderDescriptor LD;
      LD.MaxLeadership = LeaderData["leadership"].GetUint();
      LD.MaxSteps = LeaderData["move_points"].GetUint();
      LD.ViewRange = LeaderData["view_range"].GetUint();
      // TODO LD.Perks
      U.LeaderDescriptorId = M.LeaderDescriptors_.AddObject(std::string{Name}, std::move(LD));
    }

    M.UnitDescriptors_.AddObject(std::move(Name), std::move(U));
  }

  static void LoadResources(Mod &M, const std::filesystem::path &Path) noexcept {
    // The order is important and is specified in resources.json.
    std::ifstream DescrFile{Path / "resources.json"};
    rapidjson::IStreamWrapper JsonStream{DescrFile};
    rapidjson::Document Doc;
    Doc.ParseStream(JsonStream);
    for (const auto &V : Doc.GetArray()) {
      LoadResource(M, Path / GetString(V));
    }
  }

  static void LoadResource(Mod &M, const std::filesystem::path &Path) noexcept {
    std::ifstream DescrFile{Path / "resource.json"};
    rapidjson::IStreamWrapper JsonStream{DescrFile};
    rapidjson::Document Doc;
    Doc.ParseStream(JsonStream);

    Named Named = LoadNamed(Doc);
    Resource R{std::move(Named)};
    std::string Name{R.GetName()};
    M.Resources_.AddObject(std::move(Name), std::move(R));
  }

  static void LoadSpells(Mod &M, const std::filesystem::path &Path) noexcept {
    LoadDirectory(M, Path, "spell.json", LoadSpell);
  }

  static void LoadSpell(Mod &M, const std::filesystem::path &Path,
                        const rapidjson::Document &Doc) noexcept {
    Named Named = LoadNamed(Doc);
    Effect E; // TODO: Parse and load
    std::string Name{Named.GetName()};
    Spell S{std::move(Named), M.GetResources()};
    S.Level = Doc["level"].GetUint();
    S.SpellEffect = E;
    const auto &Costs = Doc["costs"];
    S.LearningCost = ParseResources(M, Costs["learning"]);
    S.UseCost = ParseResources(M, Costs["use"]);
    S.TradeCost = ParseResources(M, Costs["trade"]);

    M.Resources_.AddObject(std::move(Name), std::move(S));
  }
};

Mod Mod::Load(const std::filesystem::path &Path) noexcept { return ModLoader::Load(Path); }
Mod Mod::Load(const std::string &Name) noexcept { return ModLoader::Load(Name); }

} // namespace NotAGame

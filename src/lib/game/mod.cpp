#include "game/mod.h"
#include "util/settings.h"

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include <fstream>

namespace NotAGame {

namespace {

template <typename Value> const auto *FindMemberPtr(const Value &V, const auto &Key) noexcept {
  auto Found = V.FindMember(Key);
  return Found != V.MemberEnd() ? &Found->value : nullptr;
}

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

template <typename Value>
LandPropagationSettings LoadLandPropagationSettings(const Value &V) noexcept {
  LandPropagationSettings S;
  S.TownPropagationByLevel.push_back(0);
  for (const auto &E : V["town"].GetArray()) {
    S.TownPropagationByLevel.push_back(E.GetUint());
  }
  S.CapitalPropagation = V["capital"].GetUint();
  S.RodPropagation = V["rod"].GetUint();
  return S;
}

template <typename Value>
VisibilityRangeSettings LoadVisibilityRangeSettings(const Value &V) noexcept {
  VisibilityRangeSettings S;
  S.TownVisibilityByLevel.push_back(0);
  for (const auto &E : V["town"].GetArray()) {
    S.TownVisibilityByLevel.push_back(E.GetUint());
  }
  S.CapitalVisibility = V["capital"].GetUint();
  return S;
}

template <typename Value>
Resources ParseResources(const ResourceRegistry &Registry, const Value &Doc) noexcept {
  const auto &Map = Doc.GetObject();
  Resources R{Registry};
  for (const auto &Member : Map) {
    const auto Id = Registry.GetId(GetString(Member.name));
    R.SetAmountById(Id, Member.value.GetInt());
  }
  return R;
}

template <typename Value> Resources ParseResources(const Mod &M, const Value &Doc) noexcept {
  return ParseResources(M.GetResources(), Doc);
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

    M.GridSettings_ = LoadGridSettings(Doc["grid_settings"]);
    M.InterfaceSettings_ = LoadInterfaceSettings(Doc["interface_settings"]);
    M.LandPropagationSettings_ = LoadLandPropagationSettings(Doc["land_propagation_settings"]);

    LoadBuildingPages(M, Doc["building_pages"]);
    LoadResources(M, Path / "resources");
    LoadUnitPresets(M, Path / "units");
    LoadSpells(M, Path / "spells");
    LoadFractions(M, Path / "fractions");

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

  static void LoadUnitPresets(Mod &M, const std::filesystem::path &Path) noexcept {
    LoadDirectory(M, Path, "unit.json", LoadUnitPreset);
  }

  template <typename Value> static void LoadBuildingPages(Mod &M, const Value &V) noexcept {
    for (const auto &Page : V.GetArray()) {
      auto Named = LoadNamed(Page);
      BuildingPage P{std::move(Named)};
      std::string Name = P.GetName();
      M.BuildingPages_.AddObject(std::move(Name), std::move(P));
    }
  }

  static void LoadUnitPreset(Mod &M, const std::filesystem::path &Path,
                             const rapidjson::Document &Doc) noexcept {
    Named Named = LoadNamed(Doc);
    auto Name = Named.GetName();

    Unit U{std::move(Named), M.GetResources()};

    const auto &Costs = Doc["costs"];
    U.HireCost = ParseResources(M, Costs["hire"]);
    U.ResurrectCost = ParseResources(M, Costs["resurrect"]);
    U.HealPerHPCost = ParseResources(M, Costs["heal"]);

    U.Width = Doc["width"].GetUint();
    U.Height = Doc["height"].GetUint();
    U.ExpForKill = Doc["exp_for_kill"].GetUint();

    U.Armor = CappedTrait<Size>{Doc["armor"].GetUint(), 99};
    U.Experience = Doc["exp"].GetUint();
    U.Health = Doc["health"].GetUint();
    U.Speed = Doc["speed"].GetUint();

    auto GridIcons = Icon::LoadNormalAndMirrored(Path / "grid.png");
    U.GridIcons.LookRight =
        M.Icons_.AddObject(Name + "_" + "grid_right", std::move(GridIcons.first));
    U.GridIcons.LookLeft =
        M.Icons_.AddObject(Name + "_" + "grid_left", std::move(GridIcons.second));

    Icon InfoIcon{Path / "info.png"};
    U.InfoIconId = M.Icons_.AddObject(Name + "_" + "info", std::move(InfoIcon));

    // TODO U.Immunes
    // TODO U.IconId
    // TODO U.Wards

    if (Doc.HasMember("prev_form")) {
      U.PreviousForm = M.GetUnitPresets().GetId(GetString(Doc["prev_form"]));
    }

    const auto &Growth = Doc["growth"][0]; // TODO
    U.DamageGrowth = Growth["damage"].GetUint();
    U.HealthGrowth = Growth["health"].GetUint();
    U.ExpForKillGrowth = Growth["exp_for_kill"].GetUint();

    const auto *LeaderDataDoc = FindMemberPtr(Doc, "leader");
    bool IsLeader = Doc.HasMember("leader");
    if (LeaderDataDoc) {
      LeaderData LD;
      LD.Leadership = (*LeaderDataDoc)["leadership"].GetUint();
      LD.Leadership.SetValue(0);
      LD.Steps = (*LeaderDataDoc)["move_points"].GetUint();
      LD.ViewRange = (*LeaderDataDoc)["view_range"].GetUint();
      // TODO LD.Perks
      U.LeaderDataId = M.LeaderPresets_.AddObject(std::string{Name}, std::move(LD));
    }

    M.UnitPresets_.AddObject(std::move(Name), std::move(U));
  }

  static void LoadResources(Mod &M, const std::filesystem::path &Path) noexcept {
    // The order is important and is specified in resources.json.
    LoadOrdered(M, Path, "resources.json", LoadResource);
  }

  template <typename ParserFn>
  static void LoadOrdered(Mod &M, const std::filesystem::path &Path, const std::string &ListName,
                          ParserFn &&Parser) noexcept {
    std::ifstream DescrFile{Path / ListName};
    rapidjson::IStreamWrapper JsonStream{DescrFile};
    rapidjson::Document Doc;
    Doc.ParseStream(JsonStream);
    for (const auto &V : Doc.GetArray()) {
      Parser(M, Path / GetString(V));
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
    Effect E; // TODO: Parse and load when implemented
    std::string Name{Named.GetName()};
    Spell S{std::move(Named), M.GetResources()};
    S.Level = Doc["level"].GetUint();
    S.SpellEffect = E;
    const auto &Costs = Doc["costs"];
    S.LearningCost = ParseResources(M, Costs["learning"]);
    S.UseCost = ParseResources(M, Costs["use"]);
    S.TradeCost = ParseResources(M, Costs["trade"]);

    M.Spells_.AddObject(std::move(Name), std::move(S));
  }

  template <typename Registry, typename Ids>
  static void FillIds(const rapidjson::Document &Doc, const Registry &Reg, const std::string &Field,
                      Ids &Id) noexcept {
    const auto &Values = Doc[Field].GetArray();
    Id.reserve(Values.Size());
    for (const auto &Value : Values) {
      Id.push_back(Reg.GetId(GetString(Value)));
    }
  }

  static void LoadFractions(Mod &M, const std::filesystem::path &Path) noexcept {
    LoadDirectory(M, Path, "fraction.json", LoadFraction);
  }

  static void LoadFraction(Mod &M, const std::filesystem::path &Path,
                           const rapidjson::Document &Doc) noexcept {
    Named Named = LoadNamed(Doc);
    Effect E; // TODO: Parse and load when implemented
    std::string Name{Named.GetName()};
    Fraction F{std::move(Named)};

    FillIds(Doc, M.GetSpells(), "spells", F.Spells);
    FillIds(Doc, M.UnitPresets_, "units", F.Units);
    FillIds(Doc, M.UnitPresets_, "leaders", F.Leaders);

    // Buildings are loaded sequentially to resolve requirement dependencies between them.
    LoadOrdered(
        M, Path / "buildings", "buildings.json", [&F](Mod &M, const std::filesystem::path &Path) {
          std::ifstream DescrFile{Path / "building.json"};
          rapidjson::IStreamWrapper JsonStream{DescrFile};
          rapidjson::Document Doc;
          Doc.ParseStream(JsonStream);

          auto Named = LoadNamed(Doc);
          std::string Name{Named.GetName()};
          Building B{std::move(Named), M.GetResources()};
          FillIds(Doc, F.Buildings, "requirements", B.Requirements);
          B.FunctionalDescription = GetString(Doc["functional_description"][Utils::DEFAULT_LANG]);
          B.Cost = ParseResources(M, Doc["cost"]);
          F.Buildings.AddObject(std::move(Name), std::move(B));
        });
    M.Fractions_.AddObject(std::move(Name), std::move(F));
  }
};

Mod Mod::Load(const std::filesystem::path &Path) noexcept { return ModLoader::Load(Path); }
Mod Mod::Load(const std::string &Name) noexcept { return ModLoader::Load(Name); }

} // namespace NotAGame

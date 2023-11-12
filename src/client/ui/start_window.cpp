#include "start_window.h"
#include "ui_start_window.h"

#include "global_map_window.h"
#include "player_setup_dialog.h"

#include "engine/engine.h"
#include "engine/player.h"
#include "util/types.h"

using namespace NotAGame;

StartWindow::StartWindow(QWidget *Parent)
    : QMainWindow{Parent}, UI_{new Ui::StartWindow}, Mod_{LoadMod()}, Map_{CreateMap()} {
  UI_->setupUi(this);
}

StartWindow::~StartWindow() { delete UI_; }

NotAGame::Mod StartWindow::LoadMod() noexcept { return NotAGame::Mod::Load(std::string{"basic"}); }

MapState StartWindow::CreateMap() noexcept {
  NotAGame::GlobalMap M(1, 16, 16);
  const auto &Terrains = Mod_.GetTerrains();
  const auto NumTerrains = Terrains.size();

  for (Size Layer = 0, LayerE = M.GetNumLayers(); Layer < LayerE; ++Layer) {
    for (Size X = 0, XE = M.GetWidth(); X < XE; ++X) {
      for (Size Y = 0, YE = M.GetHeight(); Y < YE; ++Y) {
        M.GetTile(Layer, X, Y).Terrain_ = std::rand() % NumTerrains;
      }
    }
  }

  GameplaySystems Systems{Mod_.GetResources(), 2, Dims3D{16, 16, 1}};
  MapObject CapObj{Named{"1st_capital", "Capital", "capitol"},
                   MapObject::Capital,
                   Coord3D{1, 1, 0},
                   Dims2D{5, 5},
                   Coord{4, 4},
                   false,
                   true};
  auto CapitalObjId = M.AddObject("1st_capital", std::move(CapObj));
  auto &Cap = M.GetObject(CapitalObjId.GetValue());
  CapitalComponent Comp{.FractionId = Mod_.GetFractions().GetId("mountain_clans"), .PlayerId = 0};
  Cap.CapitalTrait = M.AddCapital(Comp);
  VisibilityRange CapRange{
      .Player = 0, .Origin = Cap.GetPosition(), .OriginSize = Cap.GetSize(), .Radius = 7};
  Cap.VisibilityRangeTrait = Systems.Visibility.AddComponent(CapRange);

  const auto GoldId = Mod_.GetResources().GetId("gold");
  const auto RuneManaId = Mod_.GetResources().GetId("gold");
  Resources R{Mod_.GetResources()};
  R.SetAmountByName("gold", 100);
  R.SetAmountByName("mana_runes", 25);

  ResourceSource CapitalIncome{.Income = R, .Player = 0};
  Cap.ResourceTrait = Systems.Resources.AddComponent(CapitalIncome);

  return MapState{.GlobalMap = std::move(M), .Systems = std::move(Systems)};
}

void StartWindow::on_btnTestGame_clicked() {
  PlayerSetupDialog Dlg;
  if (Dlg.exec() != QDialog::DialogCode::Accepted) {
    return;
  }

  Player Human{.Name = Dlg.GetPlayerName().toStdString(),
               .PlayerColor = NotAGame::ColorByTurnOrder(1)};

  Engine Eng{Mod_, Map_};
  auto PlayerId = Eng.PlayerConnect(PlayerKind::Human).GetValue();
  Eng.SetPlayerId(PlayerId, Map_.GlobalMap.GetCapitals()[0].PlayerId);
  Eng.SetPlayerName(PlayerId, std::move(Human.Name));
  Eng.SetPlayerLord(PlayerId, Human.LordId);

  auto Response = Eng.PlayerReady(PlayerId);
  assert(Response.Result.IsSuccess());
  assert(Response.StartGame.has_value());
  assert(Response.StartGame->TurnOrder[0] == 0);
  assert(Response.StartGame->TurnEvent);
  // OnlineGameState State{Mod_, Map_, Response.StartGame.TurnOrder};

  auto *W = new GlobalMapWindow{Mod_, *Eng.GetOnlineState(), Eng, Human, this};
  Eng.SetEventListener(W);
  W->show();
  W->OnPlayerNewTurn(*Response.StartGame->TurnEvent);
  // this->hide();
}

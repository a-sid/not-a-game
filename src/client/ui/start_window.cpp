#include "start_window.h"
#include "ui_start_window.h"

#include "player_setup_dialog.h"

#include "engine/engine.h"
#include "engine/player.h"
#include "util/types.h"

using namespace NotAGame;

StartWindow::StartWindow(QWidget *Parent)
    : QMainWindow{Parent}, UI_{new Ui::StartWindow}, Mod_{LoadMod()}, GlobalMap_{CreateMap()} {
  UI_->setupUi(this);
}

StartWindow::~StartWindow() { delete UI_; }

NotAGame::Mod StartWindow::LoadMod() noexcept { return NotAGame::Mod::Load(std::string{"basic"}); }

NotAGame::GlobalMap StartWindow::CreateMap() noexcept {
  NotAGame::GlobalMap M(1, 16, 16);
  const auto &Terrains = Mod_.GetTerrains();
  const auto NumTerrains = Terrains.size();

  for (Size Layer = 0, LayerE = GlobalMap_.GetNumLayers(); Layer < LayerE; ++Layer) {
    for (Size X = 0, XE = GlobalMap_.GetWidth(); X < XE; ++X) {
      for (Size Y = 0, YE = GlobalMap_.GetHeight(); Y < YE; ++Y) {
        M.GetTile(Layer, X, Y).Terrain_ = std::rand() % NumTerrains;
      }
    }
  }

  auto Cap = std::make_unique<Capital>(Mod_.GetCapitalSettings(),
                                       Named{"1st_capital", "Capital", "capitol"},
                                       Mod_.GetFractions().GetId("mountain_clans"), 0, 1, 1);
  M.AddObject(0, 1, 1, "1st_capital", std::move(Cap));

  return M;
}

void StartWindow::on_btnTestGame_clicked() {
  PlayerSetupDialog Dlg;
  if (Dlg.exec() != QDialog::DialogCode::Accepted) {
    return;
  }

  Player Human{.Name = Dlg.GetPlayerName().toStdString(),
               .PlayerColor = NotAGame::ColorByTurnOrder(1)};

  Engine Eng{Mod_, GlobalMap_};
  auto PlayerId = Eng.PlayerConnect().GetValue();
  Eng.SetPlayerCapital(PlayerId, GlobalMap_.GetCapitals()[0]);
  Eng.SetPlayerName(PlayerId, std::move(Human.Name));
  Eng.SetPlayerLord(PlayerId, Human.LordId);
  Eng.PlayerReady(PlayerId);
  // Eng.StartGame();
}

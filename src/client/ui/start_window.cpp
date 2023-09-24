#include "start_window.h"
#include "ui_start_window.h"

#include "player_setup_dialog.h"

#include "engine/engine.h"
#include "engine/player.h"

StartWindow::StartWindow(QWidget *Parent) : QMainWindow(Parent), UI_(new Ui::StartWindow) {
  UI_->setupUi(this);
}

StartWindow::~StartWindow() { delete UI_; }

void StartWindow::on_btnTestGame_clicked() {
  PlayerSetupDialog Dlg;
  if (Dlg.exec() != QDialog::DialogCode::Accepted) {
    return;
  }

  NotAGame::Player Human{.Name = Dlg.GetPlayerName().toStdString(),
                         .PlayerColor = NotAGame::ColorByTurnOrder(0)};

  NotAGame::Player AI{.Name = "Neutral", .PlayerColor = NotAGame::ColorByTurnOrder(1)};
}

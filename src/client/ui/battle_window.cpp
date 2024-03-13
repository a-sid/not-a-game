#include "battle_window.h"
#include "ui_battle_window.h"

using namespace NotAGame;

BattleWindow::BattleWindow(const NotAGame::Mod &M, NotAGame::Engine &Eng,
                           NotAGame::GameplaySystems &Systems, Squad &Attacker, Squad &Defender,
                           QWidget *Parent)
    : QDialog{Parent}, UI_{std::make_unique<Ui::BattleWindow>()}, M_{M}, Eng_{Eng},
      Systems_{Systems}, Attacker_{Attacker}, Defender_{Defender} {
  UI_->setupUi(this);

  AttackerWidget_ = new SquadWidget(M, Systems.Units, &Attacker, UnitDirection::LookRight, this);
  UI_->layoutAttacker->addWidget(AttackerWidget_);

  DefenderWidget_ = new SquadWidget(M, Systems.Units, &Defender, UnitDirection::LookLeft, this);
  UI_->layoutDefender->addWidget(DefenderWidget_);
}

BattleWindow::~BattleWindow() noexcept = default;

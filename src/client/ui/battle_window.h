#pragma once

#include <QDialog>

#include "client/ui/squad_widget.h"
#include "engine/engine.h"
#include "entities/components.h"

namespace Ui {
class BattleWindow;
}

class BattleWindow : public QDialog {
  Q_OBJECT

public:
  explicit BattleWindow(const NotAGame::Mod &M, NotAGame::Engine &Eng,
                        NotAGame::GameplaySystems &Systems, NotAGame::Squad &Attacker,
                        NotAGame::Squad &Defender, QWidget *Parent = nullptr);
  ~BattleWindow() noexcept;

private:
  std::unique_ptr<Ui::BattleWindow> UI_;
  const NotAGame::Mod &M_;
  NotAGame::Engine &Eng_;
  NotAGame::GameplaySystems &Systems_;
  SquadWidget *AttackerWidget_;
  SquadWidget *DefenderWidget_;

  NotAGame::Squad &Attacker_;
  NotAGame::Squad &Defender_;
};

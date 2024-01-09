#pragma once

#include "engine/engine.h"
#include "game/mod.h"
#include "squad_widget.h"

#include <QMainWindow>

namespace Ui {
class CapitalViewWindow;
}

class CapitalViewWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit CapitalViewWindow(const NotAGame::Mod &M, NotAGame::GameplaySystems &Systems,
                             NotAGame::Engine &Engine, const NotAGame::Player &Player,
                             NotAGame::Id<NotAGame::CapitalComponent> CapitalId,
                             QWidget *Parent = nullptr);
  ~CapitalViewWindow();

public slots:
  void OnGuardSlotClick(QPoint Pos);
  void OnGarrisonSlotClick(QPoint Pos);

private:
  Ui::CapitalViewWindow *UI_;
  SquadWidget *GuardWidget_;
  SquadWidget *GarrisonWidget_;

  const NotAGame::Mod &Mod_;
  NotAGame::GameplaySystems &Systems_;
  NotAGame::Engine &Engine_;
  const NotAGame::Player &Player_;
  NotAGame::CapitalComponent *Capital_;

  NotAGame::GuardComponent *Guard_;
};

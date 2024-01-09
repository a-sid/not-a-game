#pragma once

#include <QMainWindow>

#include "engine/engine.h"
#include "engine/player.h"
#include "entities/components.h"
#include "game/mod.h"
#include "state/state.h"

namespace Ui {
class StartWindow;
}

class StartWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit StartWindow(QWidget *Parent = nullptr);
  ~StartWindow();

private slots:
  void on_btnTestGame_clicked();

private:
  NotAGame::Mod LoadMod() noexcept;
  NotAGame::MapState CreateMap() noexcept;

  Ui::StartWindow *UI_;

  NotAGame::Mod Mod_;
  NotAGame::MapState Map_;
  std::optional<NotAGame::Engine> Engine_;
  NotAGame::Player Player_;
};

#pragma once

#include <QMainWindow>

#include "entities/global_map.h"
#include "game/mod.h"

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
  NotAGame::GlobalMap CreateMap() noexcept;

  Ui::StartWindow *UI_;

  NotAGame::Mod Mod_;
  NotAGame::GlobalMap GlobalMap_;
};

#pragma once

#include "squad_widget.h"

#include <QMainWindow>

namespace Ui {
class CapitalViewWindow;
}

class CapitalViewWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit CapitalViewWindow(const NotAGame::InterfaceSettings &IfaceSettings,
                             const NotAGame::GridSettings &GridSettings, QWidget *Parent = nullptr);
  ~CapitalViewWindow();

private:
  Ui::CapitalViewWindow *UI_;

  SquadWidget *GuardWidget_;
  SquadWidget *GarrisonWidget_;
};

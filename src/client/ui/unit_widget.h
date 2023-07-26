#pragma once

#include <QWidget>

#include "entities/unit.h"
#include "game/settings.h"

namespace Ui {
class UnitWidget;
}

class UnitWidget : public QWidget {
  Q_OBJECT

public:
  UnitWidget(const NotAGame::InterfaceSettings &Settings, NotAGame::Unit *Unit,
             QWidget *Parent = nullptr);
  void Update();
  ~UnitWidget();

private:
  Ui::UnitWidget *UI_;
  const NotAGame::Unit *Unit_;
};

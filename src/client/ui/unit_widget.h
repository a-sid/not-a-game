#pragma once

#include <QWidget>

#include "entities/unit.h"

namespace Ui {
class UnitWidget;
}

struct InterfaceSettings {
  uint16_t GridSpacerHeight;
  NotAGame::Dims2D UnitGridSize;
  NotAGame::Dims2D UnitIconSize;
};

class UnitWidget : public QWidget {
  Q_OBJECT

public:
  explicit UnitWidget(const InterfaceSettings &Settings, NotAGame::Unit *Unit,
                      QWidget *Parent = nullptr);
  void Update();
  ~UnitWidget();

private:
  Ui::UnitWidget *UI_;
  const NotAGame::Unit *Unit_;
};

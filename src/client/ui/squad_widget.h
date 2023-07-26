#pragma once

#include <QWidget>

#include "unit_widget.h"

#include <entities/squad.h>

namespace Ui {
class SquadWidget;
}

class SquadWidget : public QWidget {
  Q_OBJECT

public:
  explicit SquadWidget(const InterfaceSettings &IfaceSettings,
                       const NotAGame::GridSettings &GridSettings, NotAGame::Squad *Squad,
                       QWidget *Parent = nullptr);
  ~SquadWidget();

private:
  //  Ui::SquadWidget *ui;
  NotAGame::Squad *Squad_;
  const NotAGame::GridSettings &Settings_;
};

#pragma once

#include <QWidget>

#include "unit_widget.h"

#include <entities/squad.h>

#include "game/settings.h"

namespace Ui {
class SquadWidget;
}

class SquadWidget : public QWidget {
  Q_OBJECT

public:
  SquadWidget(const NotAGame::InterfaceSettings &IfaceSettings,
              const NotAGame::GridSettings &GridSettings, NotAGame::Squad *Squad,
              QWidget *Parent = nullptr);

  void Update() noexcept;
  ~SquadWidget();

private:
  //  Ui::SquadWidget *ui;
  NotAGame::Squad *Squad_;
  const NotAGame::GridSettings &GridSettings_;
};

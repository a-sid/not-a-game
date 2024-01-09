#pragma once

#include <QWidget>

#include "unit_widget.h"

#include "entities/components.h"
#include "entities/squad.h"

#include "game/settings.h"

namespace Ui {
class SquadWidget;
}

class UnitWidget;

class SquadWidget : public QWidget {
  Q_OBJECT

public:
  SquadWidget(const NotAGame::Mod &M, NotAGame::UnitSystem &Units, NotAGame::Squad *Squad,
              QWidget *Parent = nullptr);
  ~SquadWidget();

  void SetSquad(NotAGame::Squad *Squad) noexcept;
  void Update() noexcept;

signals:
  void UnitSlotClicked(QPoint Pos);

public slots:
  void OnChildClick(QPoint Pos);

private:
  //  Ui::SquadWidget *ui;
  const NotAGame::Mod &Mod_;
  NotAGame::Squad *Squad_;
  const NotAGame::GridSettings &GridSettings_;
  const NotAGame::InterfaceSettings &IfaceSettings_;
  NotAGame::UnitSystem &Units_;
  NotAGame::SmallVector<UnitWidget *, 8> UnitWidgets_;
};

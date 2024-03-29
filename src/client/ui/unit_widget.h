#pragma once

#include <QLabel>
#include <QWidget>

#include "entities/unit.h"
#include "game/mod.h"

namespace Ui {
class UnitWidget;
}

class SquadWidget;

class UnitIconWidget : public QLabel {
  Q_OBJECT

public:
  UnitIconWidget(QWidget *Parent = nullptr) : QLabel{Parent} {
    setFrameStyle(QFrame::Sunken);
    QPalette Palette;
    Palette.setColor(QPalette::Window, Qt::white);
    setAutoFillBackground(true);
    setPalette(Palette);
  }

protected:
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

  // void paintEvent(QPaintEvent * /* event */) override;

signals:
  void MouseMove(QMouseEvent *Event);
  void MouseDown(QMouseEvent *Event);
  void MouseUp(QMouseEvent *Event);
  void Clicked();

private:
  struct MouseState {
    bool IsMouseDown = false;
    bool IsDrag = false;
    QPoint MouseDownPos;
  } MouseState_;
};

class UnitWidget : public QWidget {
  Q_OBJECT

public:
  UnitWidget(const NotAGame::Mod &M, QPoint Pos, NotAGame::Unit *Unit,
             SquadWidget *Parent = nullptr) noexcept;
  ~UnitWidget() noexcept;

  void SetUnit(QPoint Pos, NotAGame::Unit *Unit) noexcept;
  void Update();

  NotAGame::UnitDirection GetUnitDirection() const noexcept { return UnitDirection_; }

  void SetUnitDirection(NotAGame::UnitDirection UnitDirection) noexcept {
    if (UnitDirection_ != UnitDirection) {
      UnitDirection_ = UnitDirection;
      Update();
    }
  }

  void ChangeUnitDirection() noexcept {
    auto NewDirection = static_cast<NotAGame::UnitDirection>(!static_cast<bool>(UnitDirection_));
    SetUnitDirection(NewDirection);
  }

signals:
  void MouseMove(QMouseEvent *event);
  void MouseDown(QMouseEvent *event);
  void MouseUp(QMouseEvent *event);

  void Clicked(QPoint);

private:
  void OnIconClick();

  Ui::UnitWidget *UI_;
  const NotAGame::Mod &Mod_;
  const NotAGame::InterfaceSettings &Settings_;
  QPoint Pos_;
  const NotAGame::Unit *Unit_;
  NotAGame::UnitDirection UnitDirection_{NotAGame::UnitDirection::LookRight};
};

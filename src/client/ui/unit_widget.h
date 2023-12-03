#pragma once

#include <QLabel>
#include <QWidget>

#include "entities/unit.h"
#include "game/settings.h"

namespace Ui {
class UnitWidget;
}

class SquadWidget;

class UnitIconWidget : public QLabel {
  Q_OBJECT

  UnitIconWidget(QWidget *Parent = nullptr) : QLabel{Parent} {}

protected:
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

  // void paintEvent(QPaintEvent * /* event */) override;

signals:
  void MouseMove(QMouseEvent *event);
  void MouseDown(QMouseEvent *event);
  void MouseUp(QMouseEvent *event);
};

class UnitWidget : public QWidget {
  Q_OBJECT

public:
  UnitWidget(const NotAGame::InterfaceSettings &Settings, NotAGame::Unit *Unit,
             SquadWidget *Parent = nullptr);
  void Update();
  ~UnitWidget();

protected:
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

  // void paintEvent(QPaintEvent * /* event */) override;

signals:
  void MouseMove(QMouseEvent *event);
  void MouseDown(QMouseEvent *event);
  void MouseUp(QMouseEvent *event);

private:
  Ui::UnitWidget *UI_;
  const NotAGame::Unit *Unit_;
};

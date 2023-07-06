#pragma once

#include "entities/global_map.h"
#include "game/mod.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MapView : public QGraphicsView {
  Q_OBJECT

public:
  using QGraphicsView::QGraphicsView;

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

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *Parent = nullptr) noexcept;
  ~MainWindow() noexcept;

public slots:
  void OnMapMouseMove(QMouseEvent *ev);
  void OnMapMouseDown(QMouseEvent *ev);
  void OnMapMouseUp(QMouseEvent *ev);

private:
  struct MapMouseState {
    bool IsDrag = false;
    QPoint MouseDownPos;
    QPoint ScrollingPos;
  };

  NotAGame::Mod LoadMod() noexcept;
  NotAGame::GlobalMap CreateMap() noexcept;
  void DrawMap() noexcept;
  void DrawTile(QPixmap &Pixmap, QPainter &Painter, int X, int Y,
                NotAGame::Size MapHeight) noexcept;

  Ui::MainWindow *UI_;
  QGraphicsScene Scene_;
  NotAGame::Mod Mod_;
  NotAGame::GlobalMap GlobalMap_;
  std::vector<QBrush> Brushes_;

  QPixmap MapPixmap_;

  MapMouseState MapMouseState_;
  NotAGame::Dim CurrentLayer_ = 0;
};

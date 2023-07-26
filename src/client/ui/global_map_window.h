#pragma once

#include "entities/global_map.h"
#include "game/mod.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class GlobalMapWindow;
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

class GlobalMapWindow : public QMainWindow {
  Q_OBJECT

public:
  GlobalMapWindow(QWidget *Parent = nullptr) noexcept;
  ~GlobalMapWindow() noexcept;

public slots:
  void OnMapMouseMove(QMouseEvent *ev);
  void OnMapMouseDown(QMouseEvent *ev);
  void OnMapMouseUp(QMouseEvent *ev);
  void OpenCapitalScreen();

private:
  struct MapMouseState {
    bool IsMouseDown = false;
    bool IsDrag = false;
    QPoint MouseDownPos;
    QPoint ScrollingPos;
  };

  NotAGame::Mod LoadMod() noexcept;
  NotAGame::GlobalMap CreateMap() noexcept;
  void DrawMap() noexcept;
  void DrawRect(QPainter &Painter, QPen Pen, QBrush Brush, int X, int Y, int Width,
                int Height) noexcept;
  void DrawTile(QPainter &Painter, int X, int Y) noexcept;
  void DrawObject(QPainter &Painter, const NotAGame::MapObject &Object) noexcept;

  void HandleObjectClick(QPoint MapCoord, NotAGame::Id<NotAGame::MapObjectPtr> Object) noexcept;

  std::optional<QPoint> GetMapCoord(QPoint MousePos) const noexcept;

  Ui::GlobalMapWindow *UI_;
  QGraphicsScene Scene_;
  NotAGame::Mod Mod_;
  NotAGame::GlobalMap GlobalMap_;
  std::vector<QBrush> Brushes_;

  QPixmap MapPixmap_;

  MapMouseState MapMouseState_;
  NotAGame::Dim CurrentLayer_ = 0;
};

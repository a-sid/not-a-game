#pragma once

#include "engine/engine.h"
#include "engine/path.h"
#include "engine/player.h"
#include "entities/global_map.h"
#include "game/mod.h"
#include "resources_widget.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLabel>
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

class GlobalMapWindow : public QMainWindow, public NotAGame::EventListener {
  Q_OBJECT

public:
  GlobalMapWindow(NotAGame::Mod &Mod, NotAGame::OnlineGameState &State, NotAGame::Engine &Engine,
                  NotAGame::Player &Player, QWidget *Parent = nullptr) noexcept;
  ~GlobalMapWindow() noexcept;

  void OnPlayerNewTurn(const NotAGame::NewTurnEvent &Event) noexcept override;

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

  void DrawMap() noexcept;
  void DrawRect(QPainter &Painter, QPen Pen, QBrush Brush, int X, int Y, int Width,
                int Height) noexcept;
  void DrawTile(QPainter &Painter, int X, int Y) noexcept;
  void DrawObject(QPainter &Painter, const NotAGame::MapObject &Object) noexcept;
  void DrawPath(QPainter &Painter, const NotAGame::Path &Path) noexcept;

  void HandleTileClick(QPoint MapCoord, const NotAGame::Tile &Tile) noexcept;
  void HandleObjectClick(QPoint MapCoord, NotAGame::Id<NotAGame::MapObject> Object) noexcept;
  void HandleSquadClick(QPoint MapCoord, NotAGame::Id<NotAGame::Squad> Squad) noexcept;
  bool TrySelect(QPoint MapCoord, NotAGame::Id<NotAGame::MapObject> ObjectId) noexcept;
  bool TrySelectSquad(NotAGame::Id<NotAGame::Squad> SquadId) noexcept;

  std::optional<QPoint> GetMapCoord(QPoint MousePos) const noexcept;

  Ui::GlobalMapWindow *UI_;
  QGraphicsScene Scene_;
  ResourcesWidget *Resources_;
  NotAGame::Mod &Mod_;
  NotAGame::OnlineGameState &State_;
  NotAGame::GlobalMap &GlobalMap_;
  NotAGame::Engine &Engine_;
  NotAGame::Player &Player_;
  std::vector<QBrush> Brushes_;

  std::unordered_map<NotAGame::Id<NotAGame::Squad>, NotAGame::Path> SquadPath_;

  QPixmap MapPixmap_;

  MapMouseState MapMouseState_;
  std::variant<std::monostate, NotAGame::Id<NotAGame::MapObject>, NotAGame::Id<NotAGame::Squad>>
      SelectedObject_;
  NotAGame::Dim CurrentLayer_ = 0;
};

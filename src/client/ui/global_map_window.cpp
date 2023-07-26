#include "global_map_window.h"
#include "./ui_global_map_window.h"
#include "capital_view_window.h"

#include <cmath>
#include <fmt/format.h>

#include <QMouseEvent>
#include <QScrollBar>

#include <QtDebug>

using namespace NotAGame;

namespace {

constexpr int kCellSize = 50;
const float kCos60 = std::cos(M_PI / 3);
const float kSin60 = std::sin(M_PI / 3);
const float kDY = kCellSize * kCos60;
const float kDX = kCellSize * kSin60;
constexpr int kBorderWidth = 200;

} // namespace

GlobalMapWindow::GlobalMapWindow(QWidget *Parent) noexcept
    : QMainWindow{Parent}, UI_{new Ui::GlobalMapWindow}, Mod_{LoadMod()}, GlobalMap_{CreateMap()} {
  UI_->setupUi(this);

  UI_->GlobalMapView->setRenderHint(QPainter::Antialiasing);
  const auto W = GlobalMap_.GetWidth(), H = GlobalMap_.GetHeight();
  MapPixmap_ = QPixmap{static_cast<int>((W + H) * kDX + kBorderWidth * 2),
                       static_cast<int>((W + H) * kDY) + kBorderWidth * 2};

  connect(UI_->GlobalMapView, &MapView::MouseMove, this, &GlobalMapWindow::OnMapMouseMove);
  connect(UI_->GlobalMapView, &MapView::MouseDown, this, &GlobalMapWindow::OnMapMouseDown);
  connect(UI_->GlobalMapView, &MapView::MouseUp, this, &GlobalMapWindow::OnMapMouseUp);
  connect(UI_->btnCapital, &QPushButton::clicked, this, &GlobalMapWindow::OpenCapitalScreen);

  QBrush WaterBrush{Qt::cyan, Qt::SolidPattern};
  QBrush GroundBrush{QColor{160, 183, 30}, Qt::SolidPattern};
  QBrush RoadBrush{Qt::darkYellow, Qt::SolidPattern};
  QBrush ForestBrush{Qt::darkGreen, Qt::SolidPattern};

  const auto &Terrains = Mod_.GetTerrains();
  Brushes_.resize(Terrains.size());
  Brushes_[*Terrains.TryGetId("water")] = WaterBrush;
  Brushes_[*Terrains.TryGetId("ground")] = GroundBrush;
  Brushes_[*Terrains.TryGetId("road")] = RoadBrush;
  Brushes_[*Terrains.TryGetId("forest")] = ForestBrush;

  DrawMap();
}

GlobalMapWindow::~GlobalMapWindow() noexcept { delete UI_; }

NotAGame::Mod GlobalMapWindow::LoadMod() noexcept {
  return NotAGame::Mod::Load(std::string{"basic"});
}

NotAGame::GlobalMap GlobalMapWindow::CreateMap() noexcept {
  NotAGame::GlobalMap M(1, 16, 16);
  const auto &Terrains = Mod_.GetTerrains();
  const auto NumTerrains = Terrains.size();

  for (Size Layer = 0, LayerE = GlobalMap_.GetNumLayers(); Layer < LayerE; ++Layer) {
    for (Size X = 0, XE = GlobalMap_.GetWidth(); X < XE; ++X) {
      for (Size Y = 0, YE = GlobalMap_.GetHeight(); Y < YE; ++Y) {
        M.GetTile(Layer, X, Y).Terrain_ = std::rand() % NumTerrains;
      }
    }
  }

  auto Cap = std::make_unique<Capital>(Mod_.GetCapitalSettings(),
                                       Named{"1st_capital", "Capital", "capitol"}, 0, 1, 1);
  M.AddObject(0, 1, 1, "1st_capital", std::move(Cap));

  return M;
}

void GlobalMapWindow::DrawRect(QPainter &Painter, QPen Pen, QBrush Brush, int X, int Y, int Width,
                               int Height) noexcept {
  auto MapHeight = GlobalMap_.GetHeight();
  Painter.save();
  Painter.translate(kBorderWidth + (MapHeight + X - Y) * kDX, kBorderWidth + (X + Y) * kDY);

  Painter.setBrush(Brush);
  Painter.setPen(Pen);
  QPointF Points[] = {{0, 0},
                      {kDX * Width, kDY * Width},
                      {kDX * (Width - Height), kDY * (Width + Height)},
                      {-Height * kDX, Height * kDY}};
  Painter.drawPolygon(Points, std::size(Points));
  Painter.restore();
}

void GlobalMapWindow::DrawTile(QPainter &Painter, int X, int Y) noexcept {
  const auto &Tile = GlobalMap_.GetTile(CurrentLayer_, X, Y);
  QBrush Brush =
      Tile.Terrain_.IsValid() ? Brushes_[Tile.Terrain_] : QBrush{Qt::gray, Qt::SolidPattern};
  DrawRect(Painter, QPen{Qt::yellow}, Brush, X, Y, 1, 1);
}

void GlobalMapWindow::DrawObject(QPainter &Painter, const MapObject &Object) noexcept {
  switch (Object.GetKind()) {
  case MapObject::Kind::Capital: {
    DrawRect(Painter, QPen{Qt::red, 2}, QBrush{Qt::darkRed}, Object.GetX(), Object.GetY(),
             Object.GetWidth(), Object.GetHeight());
    break;
  }
  default:
    assert(false && "Unsupported kind!");
  }
}

std::optional<QPoint> GlobalMapWindow::GetMapCoord(QPoint MousePos) const noexcept {
  int MapViewX = MousePos.x() + UI_->GlobalMapView->horizontalScrollBar()->value();
  int MapViewY = MousePos.y() + UI_->GlobalMapView->verticalScrollBar()->value();

  float T = (MapViewX - kBorderWidth) / kDX;
  float U = (MapViewY - kBorderWidth) / kDY;
  const auto W = GlobalMap_.GetWidth(), H = GlobalMap_.GetHeight();
  int MapX = (T + U - H) / 2, MapY = (U - T + H) / 2;
  if (MapX < 0 || MapX >= W || MapY < 0 || MapY >= H) {
    return std::nullopt;
  }
  return QPoint{MapX, MapY};
}

void GlobalMapWindow::DrawMap() noexcept {
  Scene_.clear();
  MapPixmap_.fill(Qt::black);
  QPainter Painter{&MapPixmap_};
  Painter.setRenderHint(QPainter::Antialiasing, true);
  for (Size X = 0, XE = GlobalMap_.GetWidth(); X < XE; ++X) {
    for (Size Y = 0, YE = GlobalMap_.GetHeight(); Y < YE; ++Y) {
      DrawTile(Painter, X, Y);
    }
  }
  for (auto ObjId : GlobalMap_.GetObjectsOnLayer(CurrentLayer_)) {
    DrawObject(Painter, GlobalMap_.GetObject(ObjId));
  }
  Scene_.addPixmap(MapPixmap_);
  UI_->GlobalMapView->setScene(&Scene_);
}

void GlobalMapWindow::OnMapMouseMove(QMouseEvent *Event) {
  if (MapMouseState_.IsMouseDown) {
    MapMouseState_.IsDrag = true;
    qDebug() << MapMouseState_.ScrollingPos.x() << ", " << Event->x() << ", "
             << MapMouseState_.MouseDownPos.x();
    UI_->GlobalMapView->horizontalScrollBar()->setValue(
        MapMouseState_.ScrollingPos.x() - Event->x() + MapMouseState_.MouseDownPos.x());
    UI_->GlobalMapView->verticalScrollBar()->setValue(MapMouseState_.ScrollingPos.y() - Event->y() +
                                                      MapMouseState_.MouseDownPos.y());
  }

  DrawMap();

  int MapViewX = Event->x() + UI_->GlobalMapView->horizontalScrollBar()->value();
  int MapViewY = Event->y() + UI_->GlobalMapView->verticalScrollBar()->value();

  const auto MapCoord = GetMapCoord({Event->x(), Event->y()});
  if (!MapCoord) {
    return;
  }
  const auto MapX = MapCoord->x(), MapY = MapCoord->y();

  const auto W = GlobalMap_.GetWidth(), H = GlobalMap_.GetHeight();
  float Coef = 0.6;
  float Left = kBorderWidth + (H + MapX - MapY - Coef) * kDX,
        Top = kBorderWidth + (MapX + MapY - Coef + 1) * kDY;

  Scene_.addEllipse(Left, Top, 2 * Coef * kDX, 2 * Coef * kDY, QPen{Qt::blue});

  const auto Terrain =
      Mod_.GetTerrains().GetObjectById(GlobalMap_.GetTile(CurrentLayer_, MapX, MapY).Terrain_);
  UI_->statusbar->showMessage(
      QString::fromStdString(fmt::format("{}, {}     {}, {}  {}  {}", MapViewX, MapViewY, MapX,
                                         MapY, Terrain.GetName(), Terrain.GetDescription())));
}

void GlobalMapWindow::OnMapMouseDown(QMouseEvent *Event) {
  MapMouseState_.IsMouseDown = true;
  MapMouseState_.MouseDownPos = Event->pos();
  MapMouseState_.ScrollingPos = {UI_->GlobalMapView->horizontalScrollBar()->value(),
                                 UI_->GlobalMapView->verticalScrollBar()->value()};
}

void GlobalMapWindow::OnMapMouseUp(QMouseEvent *Event) {
  if (!MapMouseState_.IsDrag) { // click
    qDebug() << "Click\n";
    if (const auto MapCoord = GetMapCoord({Event->x(), Event->y()})) {
      if (auto Obj = GlobalMap_.GetTile(CurrentLayer_, MapCoord->x(), MapCoord->y()).Object_;
          Obj.IsValid()) {
        HandleObjectClick(*MapCoord, Obj);
      }
    }
  }
  MapMouseState_.IsDrag = false;
  MapMouseState_.IsMouseDown = false;
}

void GlobalMapWindow::HandleObjectClick(QPoint MapCoord, Id<MapObjectPtr> ObjectId) noexcept {
  const auto &Obj = GlobalMap_.GetObject(ObjectId);
  if (const auto Entrance = Obj.GetEntrancePos()) {
    if (MapCoord.x() == Obj.GetX() + Entrance->X && MapCoord.y() == Obj.GetY() + Entrance->Y) {
      // OnEntranceClick(Obj);
    }
    switch (Obj.GetKind()) {
    case MapObject::Kind::Capital:
      OpenCapitalScreen();
      break;
    default:
      break;
    }
  }
}

void GlobalMapWindow::OpenCapitalScreen() {
  auto W = new CapitalViewWindow{this};
  W->show();
}

void MapView::mouseMoveEvent(QMouseEvent *event) {
  emit MouseMove(event);
  QGraphicsView::mouseMoveEvent(event);
}

void MapView::mousePressEvent(QMouseEvent *event) {
  emit MouseDown(event);
  QGraphicsView::mouseMoveEvent(event);
}

void MapView::mouseReleaseEvent(QMouseEvent *event) {
  emit MouseUp(event);
  QGraphicsView::mouseReleaseEvent(event);
}

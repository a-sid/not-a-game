#include "mainwindow.h"
#include "./ui_mainwindow.h"

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

MainWindow::MainWindow(QWidget *Parent) noexcept
    : QMainWindow{Parent}, UI_{new Ui::MainWindow}, Mod_{LoadMod()}, GlobalMap_{CreateMap()} {
  UI_->setupUi(this);

  UI_->GlobalMapView->setRenderHint(QPainter::Antialiasing);
  const auto W = GlobalMap_.GetWidth(), H = GlobalMap_.GetHeight();
  MapPixmap_ = QPixmap{static_cast<int>((W + H) * kDX + kBorderWidth * 2),
                       static_cast<int>((W + H) * kDY) + kBorderWidth * 2};

  connect(UI_->GlobalMapView, &MapView::MouseMove, this, &MainWindow::OnMapMouseMove);
  connect(UI_->GlobalMapView, &MapView::MouseDown, this, &MainWindow::OnMapMouseDown);
  connect(UI_->GlobalMapView, &MapView::MouseUp, this, &MainWindow::OnMapMouseUp);

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

MainWindow::~MainWindow() noexcept { delete UI_; }

NotAGame::Mod MainWindow::LoadMod() noexcept { return NotAGame::Mod::Load(std::string{"basic"}); }

NotAGame::GlobalMap MainWindow::CreateMap() noexcept {
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

  return M;
}

void MainWindow::DrawTile(QPixmap &Pixmap, QPainter &Painter, int X, int Y,
                          Size MapHeight) noexcept {
  Painter.save();
  Painter.translate(kBorderWidth + (MapHeight + X - Y) * kDX, kBorderWidth + (X + Y) * kDY);

  const auto &Tile = GlobalMap_.GetTile(CurrentLayer_, X, Y);
  QBrush Brush =
      Tile.Terrain_.IsValid() ? Brushes_[Tile.Terrain_] : QBrush{Qt::gray, Qt::SolidPattern};
  Painter.setBrush(Brush);
  Painter.setPen(Qt::yellow);
  QPointF Points[] = {{0, 0}, {kDX, kDY}, {0, 2 * kDY}, {-kDX, kDY}};
  Painter.drawPolygon(Points, std::size(Points));
  Painter.restore();
}

void MainWindow::DrawMap() noexcept {
  Scene_.clear();
  MapPixmap_.fill(Qt::black);
  QPainter Painter{&MapPixmap_};
  Painter.setRenderHint(QPainter::Antialiasing, true);
  for (Size X = 0, XE = GlobalMap_.GetWidth(); X < XE; ++X) {
    for (Size Y = 0, YE = GlobalMap_.GetHeight(); Y < YE; ++Y) {
      DrawTile(MapPixmap_, Painter, X, Y, GlobalMap_.GetHeight());
    }
  }
  Scene_.addPixmap(MapPixmap_);
  UI_->GlobalMapView->setScene(&Scene_);
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

void MainWindow::OnMapMouseMove(QMouseEvent *Event) {
  if (MapMouseState_.IsDrag) {
    qDebug() << MapMouseState_.ScrollingPos.x() << ", " << Event->x() << ", "
             << MapMouseState_.MouseDownPos.x();
    UI_->GlobalMapView->horizontalScrollBar()->setValue(
        MapMouseState_.ScrollingPos.x() - Event->x() + MapMouseState_.MouseDownPos.x());
    UI_->GlobalMapView->verticalScrollBar()->setValue(MapMouseState_.ScrollingPos.y() - Event->y() +
                                                      MapMouseState_.MouseDownPos.y());
  }

  int MapViewX = Event->x() + UI_->GlobalMapView->horizontalScrollBar()->value();
  int MapViewY = Event->y() + UI_->GlobalMapView->verticalScrollBar()->value();

  float T = (MapViewX - kBorderWidth) / kDX;
  float U = (MapViewY - kBorderWidth) / kDY;
  const auto W = GlobalMap_.GetWidth(), H = GlobalMap_.GetHeight();
  int MapX = (T + U - H) / 2, MapY = (U - T + H) / 2;

  DrawMap();
  if (MapX < 0 || MapX >= W || MapY < 0 || MapY >= H) {
    return;
  }
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

void MainWindow::OnMapMouseDown(QMouseEvent *Event) {
  MapMouseState_.IsDrag = true;
  MapMouseState_.MouseDownPos = Event->pos();
  MapMouseState_.ScrollingPos = {UI_->GlobalMapView->horizontalScrollBar()->value(),
                                 UI_->GlobalMapView->verticalScrollBar()->value()};
}

void MainWindow::OnMapMouseUp(QMouseEvent *Event) { MapMouseState_.IsDrag = false; }

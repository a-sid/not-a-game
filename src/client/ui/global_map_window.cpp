#include "global_map_window.h"
#include "./ui_global_map_window.h"
#include "capital_view_window.h"
#include "new_turn_dialog.h"

#include "util/map_utils.h"

#include <cmath>
#include <fmt/format.h>

#include <QMouseEvent>
#include <QPainterPath>
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

GlobalMapWindow::GlobalMapWindow(Mod &Mod, OnlineGameState &State, Engine &Engine, Player &Player,
                                 QWidget *Parent) noexcept
    : QMainWindow{Parent}, UI_{new Ui::GlobalMapWindow}, Mod_{Mod}, State_{State},
      GlobalMap_{State.SavedState.Map.GlobalMap}, Engine_{Engine}, Player_{Player} {
  UI_->setupUi(this);

  UI_->GlobalMapView->setRenderHint(QPainter::Antialiasing);
  const auto W = GlobalMap_.GetWidth(), H = GlobalMap_.GetHeight();
  MapPixmap_ = QPixmap{static_cast<int>((W + H) * kDX + kBorderWidth * 2),
                       static_cast<int>((W + H) * kDY) + kBorderWidth * 2};

  Resources_ = new ResourcesWidget{Mod_, this};
  Resources_->setMinimumSize(Resources_->sizeHint());
  UI_->statusbar->addPermanentWidget(Resources_);

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
    const auto &Comp = State_.SavedState.Map.Systems.Capitals.GetComponent(Object.CapitalTrait);
    const auto &Guard = State_.SavedState.Map.Systems.Guards.GetComponent(Comp.GuardId);
    if (Guard.SquadId.IsValid()) {
      DrawRect(Painter, QPen{Qt::black, 2}, QBrush{Qt::darkCyan},
               Object.GetX() + Object.GetEntrancePos()->X,
               Object.GetY() + Object.GetEntrancePos()->Y, 1, 1);
    }
    break;
  }
  default:
    assert(false && "Unsupported kind!");
  }
}

void GlobalMapWindow::DrawPath(QPainter &Painter, const Path &Path) noexcept {
  const auto H = GlobalMap_.GetHeight();
  float Coef = 0.2;
  Size Cost = 0;
  bool IsFirst = true;
  for (const auto &Pt : Path.Waypoints) {

    if (IsFirst) {
      IsFirst = false;
    } else {
      Cost += Pt.Cost;
    }
    float Left = kBorderWidth + (H + Pt.Coord.X - Pt.Coord.Y - Coef) * kDX,
          Top = kBorderWidth + (Pt.Coord.X + Pt.Coord.Y - Coef + 1) * kDY;
    { // Path mark.
      Painter.setPen(QPen{Qt::white, 2});
      Painter.setBrush(QBrush{Qt::blue});
      Painter.drawEllipse(Left, Top, 2 * Coef * kDX, 2 * Coef * kDY);
    }

    { // Path cost.
      QPainterPath FontPath;

      QFont Font;
      Font.setFamily("EBGaramond");
      Font.setPixelSize(18);
      Font.setBold(true);

      FontPath.addText(Left + 3 * Coef * kDX, Top, Font, QString{"%1"}.arg(Cost));

      Painter.setPen(Qt::black);
      Painter.setBrush(Qt::white);

      Painter.drawPath(FontPath);
    }
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
  Painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
  Painter.setRenderHints(QPainter::TextAntialiasing, true);

  for (Size X = 0, XE = GlobalMap_.GetWidth(); X < XE; ++X) {
    for (Size Y = 0, YE = GlobalMap_.GetHeight(); Y < YE; ++Y) {
      DrawTile(Painter, X, Y);
    }
  }
  for (auto ObjId : GlobalMap_.GetObjectsOnLayer(CurrentLayer_)) {
    DrawObject(Painter, GlobalMap_.GetObject(ObjId));
  }
  if (const auto *SelectedSquad = std::get_if<Id<Squad>>(&SelectedObject_)) {
    if (const auto *Path = Utils::MapFindPtr(SquadPath_, *SelectedSquad)) {
      DrawPath(Painter, *Path);
    }
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

  const auto &Tile = GlobalMap_.GetTile(CurrentLayer_, MapX, MapY);
  const std::string *Name = nullptr;
  const std::string *Description = nullptr;
  if (Tile.Object_.IsValid()) {
    Name = &GlobalMap_.GetObject(Tile.Object_).GetTitle();
    Description = &GlobalMap_.GetObject(Tile.Object_).GetDescription();
  } else {
    const auto &Terrain = Mod_.GetTerrains().GetObjectById(Tile.Terrain_);
    Name = &Terrain.GetName();
    Description = &Terrain.GetDescription();
  }
  UI_->statusbar->showMessage(QString::fromStdString(fmt::format(
      "{}, {}     {}, {}  {}  {}", MapViewX, MapViewY, MapX, MapY, *Name, *Description)));
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
      const auto &Tile = GlobalMap_.GetTile(CurrentLayer_, MapCoord->x(), MapCoord->y());
      if (Tile.Object_.IsValid()) {
        HandleObjectClick(*MapCoord, Tile.Object_);
      } else if (Tile.Squad_.IsValid()) {
        HandleSquadClick(*MapCoord, Tile.Squad_);
      } else {
        HandleTileClick(*MapCoord, Tile);
      }
    }
  }
  MapMouseState_.IsDrag = false;
  MapMouseState_.IsMouseDown = false;
  DrawMap();
}

bool GlobalMapWindow::TrySelectSquad(Id<Squad> SquadId) noexcept {
  if (SquadId.IsInvalid()) {
    return false;
  }
  const auto &Systems = State_.SavedState.Map.Systems;
  const auto &Squad = Systems.Squads.GetComponent(SquadId);
  const auto &LeaderUnit = Systems.Units.GetComponent(Squad.GetLeader());
  const auto &LeaderData = Systems.Leaders.GetComponent(LeaderUnit.LeaderDataId);
  UI_->lblSelection->setText(QString::fromStdString(LeaderData.Name));
  return true;
}

bool GlobalMapWindow::TrySelect(QPoint MapCoord, Id<MapObject> ObjectId) noexcept {
  const auto &Obj = GlobalMap_.GetObject(ObjectId);
  if (Obj.Owner != Player_.MapId) {
    return false;
  }
  const auto &Systems = State_.SavedState.Map.Systems;
  switch (Obj.GetKind()) {
  case MapObject::Kind::Capital:
    [[fallthrough]];
  case MapObject::Kind::Town: {
    if (MapCoord == QPoint{static_cast<int>(Obj.GetX() + Obj.GetEntrancePos()->X),
                           static_cast<int>(Obj.GetY() + Obj.GetEntrancePos()->Y)}) {
      if (TrySelectSquad(Systems.Guards.GetComponent(Obj.Guard).SquadId)) {
        return true;
      }
    }
    SelectedObject_ = ObjectId;
    UI_->lblSelection->setText(QString::fromStdString(Obj.GetTitle()));
    return true;
  }

  case MapObject::Kind::Squad: {
    assert(TrySelectSquad(Obj.SquadTrait));
    return true;
  }
  default:
    return false;
  }
}

void GlobalMapWindow::HandleTileClick(QPoint MapCoord, const Tile &Tile) noexcept {
  const auto *SelectedSquad = std::get_if<Id<Squad>>(&SelectedObject_);
  if (!SelectedSquad) {
    return;
  }
  const auto &Systems = State_.SavedState.Map.Systems;

  const auto &Squad = Systems.Squads.GetComponent(*SelectedSquad);
  auto Path = TryBuildPath(
      Squad.Position,
      Coord3D{static_cast<Size>(MapCoord.x()), static_cast<Size>(MapCoord.y()), CurrentLayer_},
      State_.SavedState.Map.GlobalMap, Systems, Mod_, Squad.GetLeader());
  assert(Path);
  SquadPath_[*SelectedSquad] = std::move(*Path);
}

void GlobalMapWindow::HandleSquadClick(QPoint MapCoord, Id<Squad> SquadId) noexcept {
  const auto *SelectedSquad = std::get_if<Id<Squad>>(&SelectedObject_);
  if (SelectedSquad && *SelectedSquad == SquadId) {
    // ShowArmyDialog();
    return;
  }
  const auto &Systems = State_.SavedState.Map.Systems;
  const auto &Squad = Systems.Squads.GetComponent(SquadId);
  if (Squad.Player_ == Player_.MapId) { // Our squad, select it.
    SelectedObject_ = SquadId;
    const auto &Leader = Systems.Units.GetComponent(Squad.GetLeader());
    const auto &LeaderData = Systems.Leaders.GetComponent(Leader.LeaderDataId);
    UI_->lblSelection->setText(QString::fromStdString(LeaderData.Name));
  }
}

void GlobalMapWindow::HandleObjectClick(QPoint MapCoord, Id<MapObject> ObjectId) noexcept {
  const auto &Obj = GlobalMap_.GetObject(ObjectId);
  const auto &Systems = State_.SavedState.Map.Systems;

  // Is it a click on the entrance guard?
  if (Obj.Owner == Player_.MapId) {
    if (const auto Entrance = Obj.GetEntrancePos()) {
      if (MapCoord.x() == Obj.GetX() + Entrance->X && MapCoord.y() == Obj.GetY() + Entrance->Y) {
        const auto Guard = Obj.Guard;
        if (Guard.IsValid()) {
          const auto Squad = Systems.Guards.GetComponent(Obj.Guard).SquadId;
          if (Squad.IsValid()) {
            HandleSquadClick(MapCoord, Squad);
            return;
          }
        }
        // OnEntranceClick(Obj);
        // return;
      }
    }

    const auto *SelectedMapObject = std::get_if<Id<MapObject>>(&SelectedObject_);
    if (SelectedMapObject && *SelectedMapObject == ObjectId) {
      // Double click, open an object menu.
      switch (Obj.GetKind()) {
      case MapObject::Kind::Capital:
        OpenCapitalScreen();
        return;
      default:
        return;
      }
      return;
    }
    if (TrySelect(MapCoord, ObjectId)) {
      return;
    }
    if (SelectedObject_.index() == 0) {
      return;
    }
  }
}

void GlobalMapWindow::OpenCapitalScreen() {

  auto W = new CapitalViewWindow{Mod_,    State_.SavedState.Map.Systems, Engine_,
                                 Player_, Id<CapitalComponent>{0},       this};
  W->show();
}

void GlobalMapWindow::OnPlayerNewTurn(const NotAGame::NewTurnEvent &Event) noexcept {
  if (Event.Player == 0) { // FIXME: pass player id from start window
    Resources_->SetValue(State_.SavedState.PlayerStates[0].ResourcesGained);
    NewTurnDialog dlg{Mod_, this};
    dlg.SetCaption(QString::fromStdString(fmt::format("Начинается {} день", Event.TurnNo)));
    assert(Event.Income);
    dlg.SetResources(*Event.Income);
    dlg.exec();
  }
}

void MapView::mouseMoveEvent(QMouseEvent *event) {
  emit MouseMove(event);
  QGraphicsView::mouseMoveEvent(event);
}

void MapView::mousePressEvent(QMouseEvent *event) {
  emit MouseDown(event);
  QGraphicsView::mousePressEvent(event);
}

void MapView::mouseReleaseEvent(QMouseEvent *event) {
  emit MouseUp(event);
  QGraphicsView::mouseReleaseEvent(event);
}

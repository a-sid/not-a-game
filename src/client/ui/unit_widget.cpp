#include "unit_widget.h"

#include "qdebug.h"
#include "squad_widget.h"
#include "ui_unit_widget.h"

#include <QMouseEvent>

void UnitIconWidget::mouseMoveEvent(QMouseEvent *event) {
  if (MouseState_.IsMouseDown) {
    MouseState_.IsDrag = true;
  }
  emit MouseMove(event);
  qDebug() << "Move\n";
  QLabel::mouseMoveEvent(event);
}

void UnitIconWidget::mousePressEvent(QMouseEvent *Event) {
  MouseState_.IsMouseDown = true;
  MouseState_.MouseDownPos = Event->pos();

  emit MouseDown(Event);
  qDebug() << "Press\n";
  QLabel::mousePressEvent(Event);
}

void UnitIconWidget::mouseReleaseEvent(QMouseEvent *Event) {
  // FIXME: At least one move event is always emitted, so we filter by pos.
  if (MouseState_.MouseDownPos == Event->pos()) { // click
    qDebug() << "UnitIconWidget Click\n";
    emit Clicked();
  }
  MouseState_.IsDrag = false;
  MouseState_.IsMouseDown = false;

  emit MouseUp(Event);
  qDebug() << "Release\n";
  QLabel::mouseReleaseEvent(Event);
}

UnitWidget::UnitWidget(const NotAGame::Mod &M, QPoint Pos, NotAGame::Unit *Unit,
                       SquadWidget *Parent)
    : QWidget(Parent),
      UI_(new Ui::UnitWidget), Mod_{M}, Settings_{M.GetInterfaceSettings()}, Pos_{Pos} {
  UI_->setupUi(this);
  SetUnit(Pos, Unit);

  connect(UI_->gfxUnit, &UnitIconWidget::Clicked, this, &UnitWidget::OnIconClick);
}

void UnitWidget::OnIconClick() { emit Clicked(Pos_); }

UnitWidget::~UnitWidget() { delete UI_; }

void UnitWidget::Update() {
  uint32_t W = Unit_ ? Unit_->Width : 1;
  uint32_t H = Unit_ ? Unit_->Height : 1;

  UI_->gfxUnit->resize(Settings_.UnitGridSize.Width * W + (W - 1) * Settings_.GridSpacerHeight,
                       Settings_.UnitGridSize.Height * H + (H - 1) * Settings_.GridSpacerHeight);
  if (Unit_) {
    const auto &Icon = Mod_.GetIcons().GetObjectById(Unit_->GridIconId).Data;
    UI_->gfxUnit->setPixmap(std::any_cast<QPixmap>(Icon));
    UI_->lblHealth->setText(
        QString{"%1/%2"}.arg(Unit_->Health.GetValue(), Unit_->Health.GetEffectiveValue()));
  } else {
    UI_->lblHealth->clear();
  }
}

void UnitWidget::SetUnit(QPoint Pos, NotAGame::Unit *Unit) noexcept {
  Pos_ = Pos;
  Unit_ = Unit;
  Update();
}

// void UnitWidget::mousePressEvent(QMouseEvent *event) {
//   qDebug() << "Pressed\n";
//   QWidget::mousePressEvent(event);
// }

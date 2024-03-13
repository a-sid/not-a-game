#include "unit_widget.h"

#include "qdebug.h"
#include "squad_widget.h"
#include "ui_unit_widget.h"

#include <QMouseEvent>

using namespace NotAGame;

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

UnitWidget::UnitWidget(const Mod &M, QPoint Pos, Unit *Unit, SquadWidget *Parent) noexcept
    : QWidget(Parent),
      UI_(new Ui::UnitWidget), Mod_{M}, Settings_{M.GetInterfaceSettings()}, Pos_{Pos} {
  UI_->setupUi(this);
  SetUnit(Pos, Unit);

  connect(UI_->gfxUnit, &UnitIconWidget::Clicked, this, &UnitWidget::OnIconClick);
}

void UnitWidget::OnIconClick() { emit Clicked(Pos_); }

UnitWidget::~UnitWidget() noexcept { delete UI_; }

void UnitWidget::Update() {
  uint32_t W = Unit_ ? Unit_->Width : 1;
  uint32_t H = Unit_ ? Unit_->Height : 1;

  UI_->gfxUnit->resize(Settings_.UnitGridSize.Width * W + (W - 1) * Settings_.GridSpacerHeight,
                       Settings_.UnitGridSize.Height * H + (H - 1) * Settings_.GridSpacerHeight);
  if (Unit_) {
    const auto IconId = UnitDirection_ == UnitDirection::LookRight ? Unit_->GridIcons.LookRight
                                                                   : Unit_->GridIcons.LookLeft;
    const auto &Icon = Mod_.GetIcons().GetObjectById(IconId).GetOpaqueData();
    const auto &Pixmap = *std::any_cast<QPixmap>(&Icon);
    UI_->gfxUnit->setPixmap(Pixmap);
    std::cerr << QString{"%1/%2"}
                     .arg(Unit_->Health.GetValue())
                     .arg(Unit_->Health.GetEffectiveValue())
                     .toStdString()
              << "\n";
    UI_->lblHealth->setText(
        QString{"%1/%2"}.arg(Unit_->Health.GetValue()).arg(Unit_->Health.GetEffectiveValue()));
  } else {
    UI_->gfxUnit->clear();
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

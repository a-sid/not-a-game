#include "unit_widget.h"

#include "qdebug.h"
#include "squad_widget.h"
#include "ui_unit_widget.h"

UnitWidget::UnitWidget(const NotAGame::InterfaceSettings &Settings, NotAGame::Unit *Unit,
                       SquadWidget *Parent)
    : Unit_{Unit}, QWidget(Parent), UI_(new Ui::UnitWidget) {
  UI_->setupUi(this);
  uint32_t W = Unit_ ? Unit_->Width : 1;
  uint32_t H = Unit_ ? Unit_->Height : 1;

  UI_->gfxUnit->resize(Settings.UnitGridSize.Width * W + (W - 1) * Settings.GridSpacerHeight,
                       Settings.UnitGridSize.Height * H + (H - 1) * Settings.GridSpacerHeight);
  Update();
}

UnitWidget::~UnitWidget() { delete UI_; }

void UnitWidget::Update() {
  if (Unit_) {
    UI_->lblHealth->setText(
        QString{"%1/%2"}.arg(Unit_->Health.GetValue(), Unit_->Health.GetEffectiveValue()));
  } else {
    UI_->lblHealth->clear();
  }
}

void UnitWidget::mousePressEvent(QMouseEvent *event) {
  qDebug() << "Pressed\n";
  QWidget::mousePressEvent(event);
}

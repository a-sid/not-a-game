#include "unit_widget.h"
#include "ui_unit_widget.h"

UnitWidget::UnitWidget(const NotAGame::InterfaceSettings &Settings, NotAGame::Unit *Unit,
                       QWidget *Parent)
    : Unit_{Unit}, QWidget(Parent), UI_(new Ui::UnitWidget) {
  UI_->setupUi(this);
  uint32_t W = Unit_ ? Unit_->GetWidth() : 1;
  uint32_t H = Unit_ ? Unit_->GetHeight() : 1;

  UI_->gfxUnit->resize(Settings.UnitGridSize.Width * W + (W - 1) * Settings.GridSpacerHeight,
                       Settings.UnitGridSize.Height * H + (H - 1) * Settings.GridSpacerHeight);
  Update();
}

UnitWidget::~UnitWidget() { delete UI_; }

void UnitWidget::Update() {
  if (Unit_) {
    UI_->lblHealth->setText(QString{"%1/%2"}.arg(Unit_->GetHealth().GetValue(),
                                                 Unit_->GetHealth().GetEffectiveValue()));
  } else {
    UI_->lblHealth->clear();
  }
}

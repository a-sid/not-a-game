#include "squad_widget.h"
#include "ui_squad_widget.h"
#include "unit_widget.h"

SquadWidget::SquadWidget(const InterfaceSettings &IfaceSettings,
                         const NotAGame::GridSettings &GridSettings, NotAGame::Squad *Squad,
                         QWidget *Parent)
    : Squad_{Squad}, Settings_{GridSettings}, QWidget(Parent) /*, ui(new Ui::SquadWidget)*/ {
  std::vector<bool> Rendered(Settings_.Width * Settings_.Height, false);
  for (uint8_t H = 0, HE = Settings_.Height; H < HE; ++H) {
    for (uint8_t W = 0, WE = Settings_.Width; W < WE; ++W) {
      if (!Rendered[H * Settings_.Width + W]) {
        NotAGame::Unit *Unit = Squad ? Squad->GetUnit(W, H) : nullptr;
        UnitWidget *Widget = new UnitWidget(IfaceSettings, Unit, this);
        const uint8_t UW = Unit ? Unit->GetWidth() : 1;
        const uint8_t UH = Unit ? Unit->GetHeight() : 1;
        for (uint8_t I = H, IE = H + UH; I < IE; ++I) {
          for (uint8_t J = W, JE = W + UW; J < JE; ++J) {
            Rendered[I * Settings_.Width + J] = true;
          }
        }
      }
    }
  }

  // ui->setupUi(this);
}

SquadWidget::~SquadWidget() { /*delete ui;*/
}

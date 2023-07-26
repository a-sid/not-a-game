#include "squad_widget.h"
#include "ui_squad_widget.h"
#include "unit_widget.h"

#include <QSizePolicy>

SquadWidget::SquadWidget(const NotAGame::InterfaceSettings &IfaceSettings,
                         const NotAGame::GridSettings &GridSettings, NotAGame::Squad *Squad,
                         QWidget *Parent)
    : Squad_{Squad}, GridSettings_{GridSettings}, QWidget(Parent) /*, ui(new Ui::SquadWidget)*/ {
  setSizePolicy(QSizePolicy{QSizePolicy::Fixed, QSizePolicy::Fixed});
  setMinimumSize(GridSettings.Width * IfaceSettings.UnitGridSize.Width +
                     (GridSettings.Width + 1) * IfaceSettings.GridSpacerHeight,
                 GridSettings.Height * IfaceSettings.UnitGridSize.Height +
                     (GridSettings.Height + 1) * IfaceSettings.GridSpacerHeight);

  std::vector<bool> Rendered(GridSettings_.Width * GridSettings_.Height, false);
  for (uint8_t H = 0, HE = GridSettings_.Height; H < HE; ++H) {
    for (uint8_t W = 0, WE = GridSettings_.Width; W < WE; ++W) {
      if (!Rendered[H * GridSettings_.Width + W]) {
        NotAGame::Unit *Unit = Squad ? Squad->GetUnit(W, H) : nullptr;
        UnitWidget *Widget = new UnitWidget(IfaceSettings, Unit, this);
        Widget->move(W * (IfaceSettings.UnitGridSize.Width + IfaceSettings.GridSpacerHeight),
                     H * (IfaceSettings.UnitGridSize.Height + IfaceSettings.GridSpacerHeight));

        const uint8_t UW = Unit ? Unit->GetWidth() : 1;
        const uint8_t UH = Unit ? Unit->GetHeight() : 1;
        for (uint8_t I = H, IE = H + UH; I < IE; ++I) {
          for (uint8_t J = W, JE = W + UW; J < JE; ++J) {
            Rendered[I * GridSettings_.Width + J] = true;
          }
        }
      }
    }
  }

  // ui->setupUi(this);
}

SquadWidget::~SquadWidget() { /*delete ui;*/
}

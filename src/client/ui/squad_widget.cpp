#include "squad_widget.h"
#include "ui_squad_widget.h"
#include "unit_widget.h"

#include <QSizePolicy>

SquadWidget::SquadWidget(const NotAGame::InterfaceSettings &IfaceSettings,
                         const NotAGame::GridSettings &GridSettings, NotAGame::UnitSystem &Units,
                         NotAGame::Squad *Squad, QWidget *Parent)
    : Squad_{Squad}, GridSettings_{GridSettings}, IfaceSettings_{IfaceSettings}, Units_{Units},
      QWidget(Parent) /*, ui(new Ui::SquadWidget)*/ {
  setSizePolicy(QSizePolicy{QSizePolicy::Fixed, QSizePolicy::Fixed});
  setMinimumSize(GridSettings.Width * IfaceSettings.UnitGridSize.Width +
                     (GridSettings.Width + 1) * IfaceSettings.GridSpacerHeight,
                 GridSettings.Height * IfaceSettings.UnitGridSize.Height +
                     (GridSettings.Height + 1) * IfaceSettings.GridSpacerHeight);

  Update();
  // ui->setupUi(this);
}

void SquadWidget::Update() noexcept {
  for (auto *Widget : UnitWidgets_) {
    delete Widget;
  }
  UnitWidgets_.clear();

  std::vector<bool> Rendered(GridSettings_.Width * GridSettings_.Height, false);
  for (uint8_t H = 0, HE = GridSettings_.Height; H < HE; ++H) {
    for (uint8_t W = 0, WE = GridSettings_.Width; W < WE; ++W) {
      if (!Rendered[H * GridSettings_.Width + W]) {
        const auto UnitId = Squad_ ? Squad_->GetUnit(W, H) : NotAGame::Id<NotAGame::Unit>{};
        auto *Unit = UnitId.IsValid() ? &Units_.GetComponent(UnitId) : nullptr;

        UnitWidget *Widget = new UnitWidget(IfaceSettings_, Unit, this);
        UnitWidgets_.push_back(Widget);
        Widget->move(W * (IfaceSettings_.UnitGridSize.Width + IfaceSettings_.GridSpacerHeight),
                     H * (IfaceSettings_.UnitGridSize.Height + IfaceSettings_.GridSpacerHeight));

        const uint8_t UW = Unit ? Unit->Width : 1;
        const uint8_t UH = Unit ? Unit->Height : 1;
        for (uint8_t I = H, IE = H + UH; I < IE; ++I) {
          for (uint8_t J = W, JE = W + UW; J < JE; ++J) {
            Rendered[I * GridSettings_.Width + J] = true;
          }
        }
      }
    }
  }
}

void SquadWidget::SetSquad(NotAGame::Squad *Squad) noexcept {
  Squad_ = Squad;
  Update();
}

SquadWidget::~SquadWidget() { /*delete ui;*/
}

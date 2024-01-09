#include "squad_widget.h"
#include "ui_squad_widget.h"
#include "unit_widget.h"

#include <QSizePolicy>
#include <qdebug.h>

SquadWidget::SquadWidget(const NotAGame::Mod &M, NotAGame::UnitSystem &Units,
                         NotAGame::Squad *Squad, QWidget *Parent)
    : Mod_{M}, Squad_{Squad}, GridSettings_{M.GetGridSettings()},
      IfaceSettings_{M.GetInterfaceSettings()}, Units_{Units},
      QWidget(Parent) /*, ui(new Ui::SquadWidget)*/ {
  for (int I = 0, E = GridSettings_.Width * GridSettings_.Height; I < E; ++I) {
    auto *Widget = new UnitWidget(Mod_, QPoint{0, 0}, nullptr, this);
    connect(Widget, &UnitWidget::Clicked, this, &SquadWidget::OnChildClick);
    UnitWidgets_.push_back(Widget);
  }
  setSizePolicy(QSizePolicy{QSizePolicy::Fixed, QSizePolicy::Fixed});
  setMinimumSize(GridSettings_.Width * IfaceSettings_.UnitGridSize.Width +
                     (GridSettings_.Width + 1) * IfaceSettings_.GridSpacerHeight,
                 GridSettings_.Height * IfaceSettings_.UnitGridSize.Height +
                     (GridSettings_.Height + 1) * IfaceSettings_.GridSpacerHeight);

  Update();
  // ui->setupUi(this);
}

void SquadWidget::Update() noexcept {
  size_t CurrentWidgetIdx = 0;
  std::vector<bool> Rendered(GridSettings_.Width * GridSettings_.Height, false);
  for (uint8_t H = 0, HE = GridSettings_.Height; H < HE; ++H) {
    for (uint8_t W = 0, WE = GridSettings_.Width; W < WE; ++W) {
      if (!Rendered[H * GridSettings_.Width + W]) {
        const auto UnitId = Squad_ ? Squad_->GetUnit(W, H) : NotAGame::Id<NotAGame::Unit>{};
        auto *Unit = UnitId.IsValid() ? &Units_.GetComponent(UnitId) : nullptr;

        auto *Widget = UnitWidgets_[CurrentWidgetIdx];
        Widget->setVisible(true);
        Widget->move(W * (IfaceSettings_.UnitGridSize.Width + IfaceSettings_.GridSpacerHeight),
                     H * (IfaceSettings_.UnitGridSize.Height + IfaceSettings_.GridSpacerHeight));
        Widget->SetUnit(QPoint(W, H), Unit);

        const uint8_t UW = Unit ? Unit->Width : 1;
        const uint8_t UH = Unit ? Unit->Height : 1;
        for (uint8_t I = H, IE = H + UH; I < IE; ++I) {
          for (uint8_t J = W, JE = W + UW; J < JE; ++J) {
            Rendered[I * GridSettings_.Width + J] = true;
          }
        }
        ++CurrentWidgetIdx;
      }
    }
  }

  for (size_t E = GridSettings_.Width * GridSettings_.Height; CurrentWidgetIdx < E;
       ++CurrentWidgetIdx) {
    UnitWidgets_[CurrentWidgetIdx]->SetUnit(QPoint{0, 0}, nullptr);
    UnitWidgets_[CurrentWidgetIdx]->setVisible(false);
  }
}

void SquadWidget::SetSquad(NotAGame::Squad *Squad) noexcept {
  Squad_ = Squad;
  Update();
}

SquadWidget::~SquadWidget() { /*delete ui;*/
}

void SquadWidget::OnChildClick(QPoint Pos) {
  qDebug() << "Unit click " << Pos;
  emit UnitSlotClicked(Pos);
}

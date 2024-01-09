#include "capital_view_window.h"
#include "ui_capital_view_window.h"

#include "hire_unit_dialog.h"

#include <QLabel>
#include <qdebug.h>

using namespace NotAGame;

CapitalViewWindow::CapitalViewWindow(const Mod &M, GameplaySystems &Systems, Engine &Engine,
                                     const Player &Player, Id<CapitalComponent> CapitalId,
                                     QWidget *Parent)
    : QMainWindow{Parent}, UI_{new Ui::CapitalViewWindow}, Mod_{M}, Systems_{Systems},
      Engine_{Engine}, Player_{Player} {
  UI_->setupUi(this);

  Capital_ = &Systems.Capitals.GetComponent(CapitalId);
  assert(Capital_->GuardId.IsValid());
  Guard_ = &Systems.Guards.GetComponent(Capital_->GuardId);
  auto *GuardSquad = Systems.Squads.GetComponentOrNull(Guard_->SquadId);

  GuardWidget_ = new SquadWidget{M, Systems.Units, GuardSquad, UI_->dockWidgetContents_3};
  GarrisonWidget_ = new SquadWidget{M, Systems.Units, nullptr, UI_->dockWidgetContents_3};
  UI_->SquadsLayout->addWidget(GuardWidget_);
  UI_->SquadsLayout->addWidget(GarrisonWidget_);

  connect(GuardWidget_, &SquadWidget::UnitSlotClicked, this, &CapitalViewWindow::OnGuardSlotClick);
}

CapitalViewWindow::~CapitalViewWindow() { delete UI_; }

void CapitalViewWindow::OnGuardSlotClick(QPoint Pos) {
  if (Guard_->SquadId.IsInvalid()) {
    const auto &Leaders = Mod_.GetFractions().GetObjectById(Player_.FractionId).Leaders;
    HireUnitDialog HireDlg{Mod_, Leaders, this};
    if (HireDlg.exec() == QDialog::Accepted) {
      auto UnitPresetId = HireDlg.GetSelectedUnit();
      if (UnitPresetId.IsInvalid()) {
        return;
      }

      auto HireResult =
          Engine_.HireLeader(Player_.MapId, Capital_->GuardId, UnitPresetId,
                             Coord{static_cast<Size>(Pos.x()), static_cast<Size>(Pos.y())});
      assert(HireResult.IsSuccess());
      auto &NewSquad = Systems_.Squads.GetComponent(HireResult.GetValue().SquadId);
      GuardWidget_->SetSquad(&NewSquad);
    }
  }
}

void CapitalViewWindow::OnGarrisonSlotClick(QPoint Pos) {
  // if ()
}

#include "hire_unit_dialog.h"
#include "ui_hire_unit_dialog.h"

using namespace NotAGame;

HireUnitDialog::HireUnitDialog(const Mod &M, const std::vector<Id<Unit>> &Units, QWidget *Parent)
    : QDialog(Parent), Mod_{M}, Units_{Units}, UI_{new Ui::HireUnitDialog} {
  UI_->setupUi(this);

  for (const auto Id : Units_) {
    const auto &Unit = Mod_.GetUnitPresets().GetObjectById(Id);
    UI_->lstUnits->addItem(QString::fromStdString(Unit.GetTitle()));
  }
}

HireUnitDialog::~HireUnitDialog() { delete UI_; }

Id<Unit> HireUnitDialog::GetSelectedUnit() const noexcept {
  const auto SelectedIndex = UI_->lstUnits->currentRow();
  if (SelectedIndex < 0) {
    return {};
  }
  assert(SelectedIndex < Units_.size());
  return Units_[SelectedIndex];
}

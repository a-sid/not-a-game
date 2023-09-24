#include "player_setup_dialog.h"
#include "ui_player_setup_dialog.h"

SelectWidgetButton::SelectWidgetButton(int index, QWidget *Parent)
    : Index_{index}, QPushButton{Parent} {
  setCheckable(true);
  setFlat(true);
  connect(this, &SelectWidgetButton::clicked, this, &SelectWidgetButton::Clicked);
}

void SelectWidgetButton::Clicked() { emit Toggled(Index_); }

SelectWidget::SelectWidget(QWidget *Parent) : QWidget{Parent} {
  Layout_ = new QHBoxLayout(this);
  setLayout(Layout_);
}

void SelectWidget::AddSwitch(const QString &Text) noexcept {
  SelectWidgetButton *Button = new SelectWidgetButton{Buttons_.size(), this};
  Button->setText(Text);
  connect(Button, SIGNAL(Toggled(int)), this, SLOT(SetSelectedIndex(int)));
  Layout_->addWidget(Button);
  Buttons_.push_back(Button);
  if (!CanDisableSelection_ && Buttons_.size() == 1) {
    SetSelectedIndex(0);
  }
}

int SelectWidget::GetSelectedIndex() const noexcept { return SelectedIndex_; }

void SelectWidget::SetSelectedIndex(int Index) noexcept {
  if (Index == -1) {
    if (!CanDisableSelection_) {
      return;
    }
    if (SelectedIndex_ != -1) {
      Buttons_[SelectedIndex_]->setChecked(false); // Override automatic toggle.
      SelectedIndex_ = Index;
      emit SelectionChanged(SelectedIndex_);
    }
    return;
  }

  assert(Index < Buttons_.size());

  if (Index == SelectedIndex_) {
    if (CanDisableSelection_) {
      SetSelectedIndex(-1);
    } else {
      Buttons_[SelectedIndex_]->setChecked(true); // Override automatic toggle.
    }
    return;
  }

  if (SelectedIndex_ != -1) {
    Buttons_[SelectedIndex_]->setChecked(false);
  }
  Buttons_[Index]->setChecked(true);
  SelectedIndex_ = Index;
  emit SelectionChanged(SelectedIndex_);
}

void SelectWidget::SetCanDisableSelection(bool CanDisableSelection) noexcept {
  CanDisableSelection_ = CanDisableSelection;
  if (!CanDisableSelection && SelectedIndex_ == -1 && !Buttons_.empty()) {
    SetSelectedIndex(0);
  }
}

PlayerSetupDialog::PlayerSetupDialog(QWidget *Parent)
    : QDialog(Parent), UI_(new Ui::PlayerSetupDialog) {
  UI_->setupUi(this);
  UI_->LordSelector->AddSwitch("Warlord");
  UI_->LordSelector->AddSwitch("Guildmaster");
  UI_->LordSelector->AddSwitch("Mage");
  UI_->LordSelector->SetCanDisableSelection(false);

  UI_->DifficultySelector->AddSwitch("Easy");
  UI_->DifficultySelector->AddSwitch("Normal");
  UI_->DifficultySelector->AddSwitch("Hard");
  UI_->DifficultySelector->AddSwitch("Very Hard");
  UI_->DifficultySelector->SetCanDisableSelection(false);

  connect(UI_->btnNext, &QPushButton::clicked, this, &QDialog::accept);
  connect(UI_->btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

int PlayerSetupDialog::GetLordIndex() const noexcept {
  return UI_->LordSelector->GetSelectedIndex();
}

PlayerSetupDialog::~PlayerSetupDialog() { delete UI_; }

QString PlayerSetupDialog::GetPlayerName() const noexcept { return UI_->txtPlayerName->text(); }

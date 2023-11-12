#include "client/ui/new_turn_dialog.h"
#include "ui_new_turn_dialog.h"

using namespace NotAGame;

NewTurnDialog::NewTurnDialog(const NotAGame::Mod &M, QWidget *Parent)
    : QDialog(Parent), UI_(new Ui::NewTurnDialog) {
  UI_->setupUi(this);
  Resources_ = new ResourcesWidget(M, this);
  UI_->layoutView->addWidget(Resources_);
  UI_->layoutView->addStretch();
}

NewTurnDialog::~NewTurnDialog() { delete UI_; }

void NewTurnDialog::SetCaption(const QString &Caption) noexcept {
  UI_->lblCaption->setText(Caption);
}

void NewTurnDialog::SetResources(const NotAGame::Resources &NewValue) noexcept {
  Resources_->SetValue(NewValue);
}

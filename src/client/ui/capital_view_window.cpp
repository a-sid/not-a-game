#include "capital_view_window.h"
#include "ui_capital_view_window.h"

#include <QLabel>
#include <qdebug.h>

CapitalViewWindow::CapitalViewWindow(const NotAGame::InterfaceSettings &IfaceSettings,
                                     const NotAGame::GridSettings &GridSettings, QWidget *Parent)
    : QMainWindow(Parent), UI_(new Ui::CapitalViewWindow) {
  UI_->setupUi(this);
  GuardWidget_ = new SquadWidget{IfaceSettings, GridSettings, nullptr, UI_->dockWidgetContents_3};
  UI_->SquadsLayout->addWidget(GuardWidget_);
}

CapitalViewWindow::~CapitalViewWindow() { delete UI_; }

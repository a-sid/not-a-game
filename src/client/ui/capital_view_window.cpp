#include "capital_view_window.h"
#include "ui_capital_view_window.h"

#include <qdebug.h>

CapitalViewWindow::CapitalViewWindow(QWidget *Parent)
    : QMainWindow(Parent), UI_(new Ui::CapitalViewWindow) {
  UI_->setupUi(this);
}

CapitalViewWindow::~CapitalViewWindow() { delete UI_; }

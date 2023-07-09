#include "capital_view_window.h"
#include "ui_capital_view_window.h"

CapitalViewWindow::CapitalViewWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CapitalViewWindow)
{
    ui->setupUi(this);
}

CapitalViewWindow::~CapitalViewWindow()
{
    delete ui;
}

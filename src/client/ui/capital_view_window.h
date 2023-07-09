#ifndef CAPITAL_VIEW_WINDOW_H
#define CAPITAL_VIEW_WINDOW_H

#include <QMainWindow>

namespace Ui {
class CapitalViewWindow;
}

class CapitalViewWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CapitalViewWindow(QWidget *parent = nullptr);
    ~CapitalViewWindow();

private:
    Ui::CapitalViewWindow *ui;
};

#endif // CAPITAL_VIEW_WINDOW_H

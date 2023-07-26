#pragma once

#include <QMainWindow>

namespace Ui {
class CapitalViewWindow;
}

class CapitalViewWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit CapitalViewWindow(QWidget *Parent = nullptr);
  ~CapitalViewWindow();

private:
  Ui::CapitalViewWindow *UI_;
};

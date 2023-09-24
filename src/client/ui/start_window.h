#pragma once

#include <QMainWindow>

namespace Ui {
class StartWindow;
}
class StartWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit StartWindow(QWidget *Parent = nullptr);
  ~StartWindow();

private slots:
  void on_btnTestGame_clicked();

private:
  Ui::StartWindow *UI_;
};

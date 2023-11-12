#pragma once

#include <QDialog>

#include "client/ui/resources_widget.h"
#include "game/mod.h"

namespace Ui {
class NewTurnDialog;
}

class NewTurnDialog : public QDialog {
  Q_OBJECT

public:
  explicit NewTurnDialog(const NotAGame::Mod &M, QWidget *Parent = nullptr);
  ~NewTurnDialog();

public slots:
  void SetCaption(const QString &Caption) noexcept;
  void SetResources(const NotAGame::Resources &NewValue) noexcept;

private:
  Ui::NewTurnDialog *UI_;
  ResourcesWidget *Resources_;
};

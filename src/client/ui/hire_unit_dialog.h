#pragma once

#include "entities/unit.h"
#include "game/mod.h"

#include <QDialog>

namespace Ui {
class HireUnitDialog;
}

class HireUnitDialog : public QDialog {
  Q_OBJECT

public:
  explicit HireUnitDialog(const NotAGame::Mod &M,
                          const std::vector<NotAGame::Id<NotAGame::Unit>> &Units,
                          QWidget *Parent = nullptr);
  NotAGame::Id<NotAGame::Unit> GetSelectedUnit() const noexcept;
  ~HireUnitDialog();

private:
  const NotAGame::Mod &Mod_;
  const std::vector<NotAGame::Id<NotAGame::Unit>> &Units_;
  Ui::HireUnitDialog *UI_;
};

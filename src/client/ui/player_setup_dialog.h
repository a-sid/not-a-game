#pragma once

#include <QDialog>
#include <QHBoxLayout>
#include <QPushButton>

namespace Ui {
class PlayerSetupDialog;
}

class SelectWidgetButton : public QPushButton {
  Q_OBJECT

public:
  explicit SelectWidgetButton(int index, QWidget *Parent = nullptr);

signals:
  void Toggled(int Index);

private slots:
  void Clicked();

private:
  int Index_;
};

class SelectWidget : public QWidget {
  Q_OBJECT

public:
  explicit SelectWidget(QWidget *Parent = nullptr);

  void AddSwitch(const QString &Text) noexcept;
  int GetSelectedIndex() const noexcept;
  void SetCanDisableSelection(bool CanDisableSelection) noexcept;

signals:
  void SelectionChanged(int);

public slots:
  void SetSelectedIndex(int Index) noexcept;

private:
  QVector<QPushButton *> Buttons_;
  QHBoxLayout *Layout_;
  int SelectedIndex_ = -1;
  bool CanDisableSelection_ = true;
};

class PlayerSetupDialog : public QDialog {
  Q_OBJECT

public:
  explicit PlayerSetupDialog(QWidget *Parent = nullptr);
  ~PlayerSetupDialog();
  int GetLordIndex() const noexcept;
  int GetComplexity() const noexcept;
  QString GetPlayerName() const noexcept;

private slots:

private:
  Ui::PlayerSetupDialog *UI_;
};

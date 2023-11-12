#pragma once

#include <QLabel>
#include <QWidget>

#include "game/mod.h"

class ResourcesWidget : public QWidget {
  Q_OBJECT

public:
  explicit ResourcesWidget(const NotAGame::Mod &M, QWidget *Parent = nullptr);
  ~ResourcesWidget();

  const NotAGame::Resources &GetValue() const noexcept { return Value_; }

public slots:
  void SetValue(const NotAGame::Resources &NewValue) noexcept;

private:
  struct ResourceView {
    QLabel *Pic;
    QLabel *Amount;
  };

  const NotAGame::ResourceRegistry &Registry_;
  NotAGame::Resources Value_;
  NotAGame::SmallVector<ResourceView, NotAGame::kExpectedResourceCount> Views_;
};

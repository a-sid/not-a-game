#include "client/ui/resources_widget.h"

#include <QHBoxLayout>
#include <QSpacerItem>

using namespace NotAGame;

ResourcesWidget::ResourcesWidget(const NotAGame::Mod &M, QWidget *Parent)
    : Registry_{M.GetResources()}, Value_{Registry_}, QWidget(Parent) {
  auto *Layout = new QHBoxLayout(this);
  setLayout(Layout);

  for (Size I = 0, E = Registry_.size(); I < E; ++I) {
    const auto &R = Registry_.GetObjectById(I);
    QLabel *Pic = new QLabel{QString::fromStdString(R.GetTitle()), this};
    Pic->setToolTip(QString::fromStdString(R.GetDescription()));
    QLabel *Amount = new QLabel{"0", this};
    Views_.push_back(ResourceView{.Pic = Pic, .Amount = Amount});

    if (I != 0) {
      Layout->addSpacing(20);
    }
    Layout->addWidget(Pic);
    Layout->addWidget(Amount);
  }
}

ResourcesWidget::~ResourcesWidget() {}

void ResourcesWidget::SetValue(const NotAGame::Resources &NewValue) noexcept {
  Value_ = NewValue;
  for (Size I = 0, E = Registry_.size(); I < E; ++I) {
    Views_[I].Amount->setText(QString::number(Value_.GetAmountById(I)));
  }
}

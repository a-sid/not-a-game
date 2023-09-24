#include "client/ui/start_window.h"

#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication App(argc, argv);
  StartWindow W;
  W.show();
  return App.exec();
}

#include "client/ui/global_map_window.h"

#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  GlobalMapWindow w;
  w.show();
  return a.exec();
}

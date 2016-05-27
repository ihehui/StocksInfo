#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  a.addLibraryPath(QCoreApplication::applicationDirPath());
  a.addLibraryPath(QCoreApplication::applicationDirPath() + "/plugins");

  MainWindow w;
  w.resize(800, 600);
  w.show();
  
  return a.exec();
}

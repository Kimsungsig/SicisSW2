#include "mainwindow.h"
#include <QApplication>
#include <qlabel.h>

int main(int argc, char* argv[])
{
   QApplication a(argc, argv);

   MainWindow w;
//   w.setWindowTitle(QString::fromUtf8("Qt Serial communication"));
//   w.setFixedSize(300, 150);
   w.show();

   return a.exec();
}

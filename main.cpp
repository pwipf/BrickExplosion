#include "glwidget.h"
#include "mainwindow.h"
#include <QApplication>
#include <QDesktopWidget>

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);

   QSurfaceFormat format;
   format.setVersion(MAJORVNUM,MINORVNUM);
   format.setProfile(QSurfaceFormat::CoreProfile);
   QSurfaceFormat::setDefaultFormat(format);

   MainWindow *w = new MainWindow;
   QDesktopWidget d;

   QRect r=d.availableGeometry(d.primaryScreen());

   w->resize(r.width()*.7,r.height()*.7);
   w->show();

   return a.exec();
}

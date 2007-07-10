#include "kdatewidget.h"
#include <QtGui/QLineEdit>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <klocale.h>

int main(int argc, char** argv)
{
  KAboutData about("KDateWidgettest", "kdelibs4", ki18n("KDateWidgettest"), "version");
  KCmdLineArgs::init(argc, argv, &about);

  KApplication app;

  KDateWidget dateWidget;
  dateWidget.show();
  // dateWidget.setEnabled(false);
  return app.exec();
}


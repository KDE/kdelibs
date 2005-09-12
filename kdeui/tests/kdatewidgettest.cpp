#include "kdatewidget.h"
#include <qlineedit.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <klocale.h>

int main(int argc, char** argv)
{
  KLocale::setMainCatalog("kdelibs");
  KAboutData about("KDateWidgettest", "KDateWidgettest", "version");
  KCmdLineArgs::init(argc, argv, &about);

  KApplication app;

  KDateWidget dateWidget;
  app.setMainWidget(&dateWidget);
  dateWidget.show();
  // dateWidget.setEnabled(false);
  return app.exec();
}


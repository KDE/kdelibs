#include "kdatetimewidget.h"
#include <kapplication.h>
#include <klocale.h>
#include <kcmdlineargs.h>

int main(int argc, char** argv)
{
  KLocale::setMainCatalog("kdelibs");
  KCmdLineArgs::init(argc, argv, "test", "Test" ,"test app" ,"1.0");
  KApplication app;
  KDateTimeWidget dateTimeWidget;
  app.setMainWidget(&dateTimeWidget);
  dateTimeWidget.show();
  // dateTimeWidget.setEnabled(false);
  return app.exec();
}

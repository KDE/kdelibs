#include "ktimewidget.h"
#include <kapplication.h>
#include <klocale.h>

int main(int argc, char** argv)
{
  KLocale::setMainCatalogue("kdelibs");
  KApplication app(argc, argv, "KTimeWidgettest");
  KTimeWidget timeWidget;
  app.setMainWidget(&timeWidget);
  timeWidget.show();
  // timeWidget.setEnabled(false);
  return app.exec();
}

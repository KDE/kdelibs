#include "kdatetimewidget.h"
#include <kapplication.h>
#include <klocale.h>

int main(int argc, char** argv)
{
  KLocale::setMainCatalogue("kdelibs");
  KApplication app(argc, argv, "KDateTimeWidgettest");
  KDateTimeWidget dateTimeWidget;
  app.setMainWidget(&dateTimeWidget);
  dateTimeWidget.show();
  // dateTimeWidget.setEnabled(false);
  return app.exec();
}

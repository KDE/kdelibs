#include "kdatewidget.h"
#include <qlineedit.h>
#include <kapplication.h>
#include <klocale.h>

int main(int argc, char** argv)
{
  KLocale::setMainCatalogue("kdelibs");
  KApplication app(argc, argv, "KDateWidgettest");
  KDateWidget dateWidget;
  app.setMainWidget(&dateWidget);
  dateWidget.show();
  // dateWidget.setEnabled(false);
  return app.exec();
}


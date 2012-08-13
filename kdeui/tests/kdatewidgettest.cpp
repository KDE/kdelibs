#include "kdatewidget.h"
#include <QApplication>
#include <QLineEdit>
#include <kaboutdata.h>
#include <klocalizedstring.h>

int main(int argc, char** argv)
{
  QApplication::setApplicationName("KDateWidgettest");

  QApplication app(argc, argv);

  KDateWidget dateWidget;
  dateWidget.show();
  // dateWidget.setEnabled(false);
  return app.exec();
}


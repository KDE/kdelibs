#include "kdatewidget.h"
#include <QApplication>

int main(int argc, char** argv)
{
  QApplication::setApplicationName("KDateWidgettest");

  QApplication app(argc, argv);

  KDateWidget dateWidget;
  dateWidget.show();
  // dateWidget.setEnabled(false);
  return app.exec();
}


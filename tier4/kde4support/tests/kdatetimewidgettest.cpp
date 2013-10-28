#include "kdatetimewidget.h"
#include <QApplication>

int main(int argc, char** argv)
{
  QApplication::setApplicationName("test");
  QApplication app(argc, argv);
  KDateTimeWidget dateTimeWidget;
  dateTimeWidget.show();
  // dateTimeWidget.setEnabled(false);
  return app.exec();
}

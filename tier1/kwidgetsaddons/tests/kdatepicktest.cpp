#include "kdatepicker.h"
#include <QApplication>
#include <QLineEdit>

int main(int argc, char** argv)
{
  QApplication::setApplicationName(QLatin1String("KDatePickertest"));

  QApplication app(argc, argv);

  KDatePicker picker;
  picker.show();
  // picker.setEnabled(false);
  return app.exec();
}


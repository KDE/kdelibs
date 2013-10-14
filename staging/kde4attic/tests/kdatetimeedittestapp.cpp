#include "kdatetimeedit.h"
#include <QApplication>

int main(int argc, char** argv)
{
  QApplication::setApplicationName("test");
  QApplication app(argc, argv);
  KDateTimeEdit dateTimeEdit;
  dateTimeEdit.setOptions(dateTimeEdit.options() | KDateTimeEdit::ShowCalendar);
  dateTimeEdit.show();
  // dateTimeEdit.setEnabled(false);
  return app.exec();
}

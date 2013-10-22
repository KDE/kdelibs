#include "kdatetimeedit.h"
#include <QApplication>

int main(int argc, char** argv)
{
  QApplication::setApplicationName(QStringLiteral("test"));
  QApplication app(argc, argv);
  KDateTimeEdit dateTimeEdit;
  dateTimeEdit.setOptions(dateTimeEdit.options()
                        | KDateTimeEdit::ShowCalendar
                        | KDateTimeEdit::SelectCalendar
                        | KDateTimeEdit::ShowTimeZone
                        | KDateTimeEdit::SelectTimeZone
                        );
  dateTimeEdit.show();
  // dateTimeEdit.setEnabled(false);
  return app.exec();
}

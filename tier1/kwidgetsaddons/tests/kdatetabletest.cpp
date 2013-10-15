#include <QApplication>

#include "kdatetable_p.h"

int main( int argc, char** argv )
{
  QApplication::setApplicationName(QLatin1String("test"));
  QApplication app(argc, argv);

  KDateTable widget;
  widget.setCustomDatePainting( QDate::currentDate().addDays(-3), QColor("green"), KDateTable::CircleMode, QColor("yellow") );
  widget.show();

  return app.exec();
}

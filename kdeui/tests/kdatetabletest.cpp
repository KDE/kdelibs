#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include "kdatetable.h"

int main( int argc, char** argv )
{
  KCmdLineArgs::init( argc, argv, "test", "kdelibs4", ki18n("Test"), "1.0", ki18n("test app"));
  KApplication app;

  KDateTable widget;
  widget.setCustomDatePainting( QDate::currentDate().addDays(-3), QColor("green"), KDateTable::CircleMode, QColor("yellow") );
  widget.show();

  return app.exec();
}

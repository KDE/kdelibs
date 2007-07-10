#include "kdatetimewidget.h"
#include <kapplication.h>
#include <klocale.h>
#include <kcmdlineargs.h>

int main(int argc, char** argv)
{
  KCmdLineArgs::init(argc, argv, "test", "kdelibs4", ki18n("Test"), "1.0", ki18n("test app"));
  KApplication app;
  KDateTimeWidget dateTimeWidget;
  dateTimeWidget.show();
  // dateTimeWidget.setEnabled(false);
  return app.exec();
}

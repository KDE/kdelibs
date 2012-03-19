#include "kdatetimewidget.h"
#include <kapplication.h>
#include <klocalizedstring.h>
#include <kcmdlineargs.h>

int main(int argc, char** argv)
{
  KCmdLineArgs::init(argc, argv, "test", "kdelibs4", qi18n("Test"), "1.0", qi18n("test app"));
  KApplication app;
  KDateTimeWidget dateTimeWidget;
  dateTimeWidget.show();
  // dateTimeWidget.setEnabled(false);
  return app.exec();
}

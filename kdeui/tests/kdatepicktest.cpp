#include "kdatepicker.h"
#include <qlineedit.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <klocale.h>

int main(int argc, char** argv)
{
  KLocale::setMainCatalog("kdelibs");
  KAboutData about("KDatePickertest", "KDatePickertest", "version");
  KCmdLineArgs::init(argc, argv, &about);

  KApplication app;

  KDatePicker picker;
  app.setMainWidget(&picker);
  picker.show();
  // picker.setEnabled(false);
  return app.exec();
}


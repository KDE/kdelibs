#include "kdatepicker.h"
#include <QLineEdit>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <klocalizedstring.h>

int main(int argc, char** argv)
{
  KAboutData about("KDatePickertest", "kdelibs4", qi18n("KDatePickertest"), "version");
  KCmdLineArgs::init(argc, argv, &about);

  KApplication app;

  KDatePicker picker;
  picker.show();
  // picker.setEnabled(false);
  return app.exec();
}


#include "kdatepik.h"
#include <qlineedit.h>
#include <kapp.h>

int main(int argc, char** argv)
{
  KApplication *app=new KApplication(argc, argv, "KDatePickertest");
  KDatePicker picker;
  app->setMainWidget(&picker);
  picker.resize(picker.sizeHint().width(), 
		 picker.sizeHint().height());
  picker.setMinimumSize(picker.size());
  picker.show();
  // picker.setEnabled(false);
  return app->exec();
}


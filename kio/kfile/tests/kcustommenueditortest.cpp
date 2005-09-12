#include "kcustommenueditor.h"
#include <kapplication.h>
#include <klocale.h>
#include <kconfig.h>

int main(int argc, char** argv)
{
  KLocale::setMainCatalog("kdelibs");
  KApplication app;
  KCustomMenuEditor editor(0);
  KConfig *cfg = new KConfig("kdesktop_custom_menu2");
  editor.load(cfg);
  if (editor.exec())
  {
     editor.save(cfg);
     cfg->sync();
  }
  delete cfg;
}


#include <klocale.h>
#include "webstyle.h"

extern "C"
{
  KStyle *      allocate()      { return new WebStyle; }
  int           minor_version() { return 0; }
  int           major_version() { return 1; }
  const char *  description()   { return(i18n("Web style plugin").toUtf8()); }
}


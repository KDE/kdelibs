#include <kapp.h>
#include <kstddirs.h>

int main(int, char **)
{
  KStandardDirs t("test");
  t.addKDEDefaults();	

  QString s = t.findResource("icon", "xv.xpm");
  if (s)
    debug(s);
}

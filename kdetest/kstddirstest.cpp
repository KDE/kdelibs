#include <kapp.h>
#include <kstddirs.h>

int main(int argc, char **argv)
{
  KStandardDirs t("test");
  t.addKDEDefaults();	

  QString s = t.findResource("icon", "xv.xpm");
  if (s)
    debug(s);
}

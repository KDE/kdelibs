#include <kapp.h>
#include <kstddirs.h>

int main(int, char **)
{
  KStandardDirs t("test");
  t.addKDEDefaults();	

  QString s = t.findResource("icon", "xv.xpm");
  if (s)
    debug(s);

  QStringList list = t.findAllResources("data", "kpanel/pics/", true);
  for (QStringList::ConstIterator it = list.begin(); it != list.end(); it++) {
    debug("data %s", (*it).ascii());
  }
}

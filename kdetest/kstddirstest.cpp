#include <kapp.h>
#include <kstddirs.h>
#include <kconfig.h>

int main(int, char **)
{
  KStandardDirs t;
  t.addKDEDefaults();	
  KConfig config; // to add custom entries - a bit tricky :/

  QString s = t.findResource("icon", "xv.xpm");
  if (s)
    debug(s);

  QStringList list = t.findAllResources("data", "kpanel/pics/", true);
  for (QStringList::ConstIterator it = list.begin(); it != list.end(); it++) {
    debug("data %s", (*it).ascii());
  }

  list = t.findAllResources("config", "kpanelrc");
  for (QStringList::ConstIterator it = list.begin(); it != list.end(); it++) {
    debug("config %s", (*it).ascii());
  }

  list = t.findAllResources("apps", "S*/*.desktop", true);
  for (QStringList::ConstIterator it = list.begin(); it != list.end(); it++) {
    debug("setting %s", (*it).ascii());
  }
  
}

#include <kapp.h>
#include <kinstance.h>
#include <kstddirs.h>
#include <kconfig.h>

int main(int argc, char **argv)
{
  KApplication a(argc, argv, "whatever", false);
  KStandardDirs t;
  t.addKDEDefaults();	
  KConfig config; // to add custom entries - a bit tricky :/

  QString s = t.findResource("icon", "xv.xpm");
  if (!s.isNull()) debug(s.ascii());

  QStringList list = t.findAllResources("data", "kfind/toolbar", true);
  for (QStringList::ConstIterator it = list.begin(); it != list.end(); it++) {
    debug("data %s", (*it).ascii());
  }

  list = t.findAllResources("config", "kcmdisplayrc");
  for (QStringList::ConstIterator it = list.begin(); it != list.end(); it++) {
    debug("config %s", (*it).ascii());
  }

  list = t.findAllResources("apps", "S*/*.desktop", true);
  for (QStringList::ConstIterator it = list.begin(); it != list.end(); it++) {
    debug("setting %s", (*it).ascii());
  }

  list = t.findDirs("data", "kwin");
  for (QStringList::ConstIterator it = list.begin(); it != list.end(); it++) {
      debug("kwin dirs %s", (*it).ascii());
  }
}

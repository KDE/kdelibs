#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kinstance.h>
#include <kstandarddirs.h>
#include <kconfig.h>

int main(int argc, char **argv)
{
   KAboutData about("kstddirstest", "kstddirstst", "version");
   KCmdLineArgs::init(argc, argv, &about);

  KApplication a;

  KStandardDirs t;
  KConfig config; // to add custom entries - a bit tricky :/

  QStringList list;
  QString s;

  t.saveLocation("icon");

  s = t.findResource("icon", "xv.xpm");
  if (!s.isNull()) kDebug() << s << endl;

  list = t.findAllResources("data", "kfind/toolbar", true);
  for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
    kDebug() << "data " << (*it).ascii() << endl;
  }

  list = t.findAllResources("config", "kcmdisplayrc");
  for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
    kDebug() << "config " << (*it).ascii() << endl;
  }

  list = t.findAllResources("config", "kcmdisplayrc", false, true);
  for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
    kDebug() << "config2 " << (*it).ascii() << endl;
  }

  list = t.findAllResources("html", "en/*/index.html", false);
  for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
    kDebug() << "docs " << (*it).ascii() << endl;
  }

  list = t.findAllResources("html", "*/*/*.html", false);
  for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
    kDebug() << "docs " << (*it).ascii() << endl;
  }

  list = t.findDirs("data", "kwin");
  for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
    kDebug() << "kwin dirs " << (*it).ascii() << endl;
  }

  kDebug() << "hit " << t.findResourceDir("config", "kcmdisplayrc") << endl;
}

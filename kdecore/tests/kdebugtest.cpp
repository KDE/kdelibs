#include <kdebug.h>
#include <kdebug2.h>
#include <kapp.h>

int main(int argc, char **argv)
{
    KApplication a(argc, argv, "kdebugtest");
    kdDebug(150) << 17 << endl;
    kdDebug() << 18 << endl;
  kDebug("Before instance creation");
  kDebugArea(1202,"Before instance creation");
  KInstance i("kdebugtest");
  kDebugInfo(150,"kDebugInfo with inexisting area number");
  kDebugInfo(1202,"This number has a value of %d",5);
  // kDebugInfo("This number should come out as appname %d %s", 5, "test");
  kDebugWarning("1+1 = %d",1+1+1);
  kDebugError(1+1 != 2, "there is something really odd!");
  kDebug("Hello world");
  QString s = "mystring";
  kDebugString(s);
  kDebugString(QString("This string is %1").arg(s));
  kDebugError(1202,"Error !!!");
  kDebugError("Error with no area");
  return 0;
}

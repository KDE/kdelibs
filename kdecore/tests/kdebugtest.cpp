#include <kdebug.h>
#include <kinstance.h>

int main()
{
  kDebug("Before instance creation");
  kDebugArea(1202,"Before instance creation");
  KInstance i("kdebugtest");
  kDebugInfo(150,"kDebugInfo with inexisting area number");
  kDebugInfo(1202,"This number has a value of %d",5);
  kDebugWarning("1+1 = %d",1+1+1);
  kDebug("Hello world");
  QString s = "mystring";
  kDebugString(s);
  kDebugString(QString("This string is %1").arg(s));
  kDebugError(1202,"Error !!!");
  kDebugError("Error with no area");
  return 0;
}

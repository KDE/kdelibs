#include <kDebug.h>
#include <kinstance.h>

// HACK !!!
#include <kdebug.h>
// HACK !!!
#define kDebugInfo( area, f...) kdebug(KDEBUG_INFO,area,##f);
#define kDebugError( area, f...) kdebug(KDEBUG_ERROR,area,##f);

int main()
{
KInstance i("blah");
kDebugInfo(150,"kDebugInfo");
kDebugInfo(1202,"This number has a value of %d",5);
kDebug("Hello world");
QString s = "mystring";
kDebugString(QString("This string is %1").arg(s));
kDebugError(0,"Error !!!");
return 0;
}

#include <kdebug.h>
#include <kinstance.h>
#include <iostream.h>

int main()
{
    kdDebug().form("mytest %s", "hello") << endl;
    QString test = "%20C this is a string";
    kdDebug(150) << test << endl;
    kdDebug() << k_lineinfo << "error on this line" << endl;
    kdDebug(2 == 2) << "this is right " << perror << endl;
    kdDebug() << "Before instance creation" << endl;
    kDebugInfo(1202,"Before instance creation");
    KInstance i("kdebugtest");
    kDebugInfo(150,"kDebugInfo with inexisting area number");
    kDebugInfo(1202,"This number has a value of %d",5);
    // kDebugInfo("This number should come out as appname %d %s", 5, "test");
    kDebugWarning("1+1 = %d",1+1+1);
    kdError(1+1 != 2) << "there is something really odd!" << endl;
    QString s = "mystring";
    kdDebug() << s << endl;
    kDebugError(1202,"Error !!!");
    kDebugError("Error with no area");
    return 0;
}

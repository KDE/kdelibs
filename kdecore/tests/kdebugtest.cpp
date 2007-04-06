#include "kdebug.h"
#include <QtGui/QWidget>
#include <kcomponentdata.h>
#include <iostream>
#include <QApplication>
#include <QtGui/QPen>
#include <QtCore/QVariant>

void testKDebug()
{
    kDebug().form("mytest %s", "hello") << endl;
    QString test = "%20C this is a string";
    kDebug(150) << test << endl;
    QByteArray cstr = test.toLatin1();
    kDebug(150) << test << endl;
    QChar ch = 'a';
    kDebug() << "QChar a: " << ch << endl;
    ch = '\r';
    kDebug() << "QChar \\r: " << ch << endl;
    kDebug() << k_lineinfo << "error on this line" << endl;
    kDebug(2 == 2) << "this is right " << perror << endl;
    kDebug() << "Before instance creation" << endl;
    kDebug(1202) << "Before instance creation" << endl;
    KComponentData i("kdebugtest");
    kDebug(1) << "kDebugInfo with inexisting area number" << endl;
    kDebug(1202) << "This number has a value of " << 5 << endl;
    // kDebug() << "This number should come out as appname " << 5 << " " << "test" << endl;
    kWarning() << "1+1 = " << 1+1+1 << endl;
    kError(1+1 != 2) << "there is something really odd!" << endl;
    QString s = "mystring";
    kDebug() << s << endl;
    kError(1202) << "Error !!!" << endl;
    kError() << "Error with no area" << endl;

    kDebug() << "Printing a null QWidget pointer: " << (QWidget*)0 << endl;

    kDebug() << "char " << '^' << " " << char(26) << endl;
    QPoint p(0,9);
    kDebug() << p << endl;

    QRect r(9,12,58,234);
    kDebug() << r << endl;

    QStringList sl;
    sl << "hi" << "this" << "list" << "is" << "short";
    kDebug() << sl << endl;

    QList<int> il;
    kDebug() << "Empty QList<int>: " << il << endl;
    il << 1 << 2 << 3 << 4 << 5;
    kDebug() << "QList<int> filled: " << il << endl;

    qint64 big = 65536LL*65536*500;
    kDebug() << big << endl;

    QVariant v( 0.12345 );
    kDebug() << "Variant: " << v << endl;

    QByteArray data;
    data.resize( 6 );
    data[0] = 42;
    data[1] = 'H';
    data[2] = 'e';
    data[3] = 'l';
    data[4] = 'l';
    data[5] = 'o';
    kDebug() << data << endl;
    data.resize( 80 );
    data.fill( 42 );
    kDebug() << data << endl;
}

int main(int, char** )
{
    testKDebug();
    return 0;
}


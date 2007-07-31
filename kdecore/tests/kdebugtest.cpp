#include "kdebug.h"
#include <QtGui/QWidget>
#include <kcomponentdata.h>
#include <iostream>
#include <QtGui/QApplication>
#include <QtGui/QPen>
#include <QtCore/QVariant>

void testKDebug()
{
    QString test = "%20C this is a string";
    kDebug(150) << test;
    QByteArray cstr = test.toLatin1();
    kDebug(150) << test;
    QChar ch = 'a';
    kDebug() << "QChar a: " << ch;
    ch = '\r';
    kDebug() << "QChar \\r: " << ch;
    kDebug() << k_lineinfo << "error on this line";
    kDebug(2 == 2) << "this is right " << perror;
    kDebug() << "Before instance creation";
    kDebug(1202) << "Before instance creation";
    KComponentData i("kdebugtest");
    kDebug(1) << "kDebugInfo with inexisting area number";
    kDebug(1202) << "This number has a value of " << 5;
    // kDebug() << "This number should come out as appname " << 5 << " " << "test";
    kWarning() << "1+1 = " << 1+1+1;
    kError(1+1 != 2) << "there is something really odd!";
    QString s = "mystring";
    kDebug() << s;
    kError(1202) << "Error !!!";
    kError() << "Error with no area";

    kDebug() << "Printing a null QWidget pointer: " << (QWidget*)0;

    kDebug() << "char " << '^' << " " << char(26);
    QPoint p(0,9);
    kDebug() << p;

    QRect r(9,12,58,234);
    kDebug() << r;

    QStringList sl;
    sl << "hi" << "this" << "list" << "is" << "short";
    kDebug() << sl;

    QList<int> il;
    kDebug() << "Empty QList<int>: " << il;
    il << 1 << 2 << 3 << 4 << 5;
    kDebug() << "QList<int> filled: " << il;

    qint64 big = 65536LL*65536*500;
    kDebug() << big;

    QVariant v( 0.12345 );
    kDebug() << "Variant: " << v;

    QByteArray data;
    data.resize( 6 );
    data[0] = 42;
    data[1] = 'H';
    data[2] = 'e';
    data[3] = 'l';
    data[4] = 'l';
    data[5] = 'o';
    kDebug() << data;
    data.resize( 80 );
    data.fill( 42 );
    kDebug() << data;
}

int main(int, char** )
{
    testKDebug();
    return 0;
}


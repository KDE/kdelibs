/* This file is part of the KDE project

   Copyright 1999-2008 David Faure <faure@kde.org>
   Copyright 2000-2005 Stephan Kulow <coolo@kde.org>
   Copyright 2007      Thiago Macieira <thiago@kde.org>

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2 of the License or
   ( at your option ) version 3 or, at the discretion of KDE e.V.
   ( which shall act as a proxy as in section 14 of the GPLv3 ), any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// to test debug messages, kdebug should be enabled regardless of build mode

#include <QCoreApplication>
#include <kcmdlineargs.h>
#include <QtCore/QDebug>
#undef QT_NO_DEBUG

#include "kdebug.h"
#include <kcomponentdata.h>
#include <iostream>
#include <QtCore/QPoint>
#include <QtCore/QRect>
#include <QtCore/QVariant>
#include <map>
#include <list>

class TestClass1
{
public:
    enum Something { foo };

    void func_void() { kDebug(); }
    int func_int() { kDebug(); return 0; }
    unsigned func_unsigned() { kDebug(); return 0; }
    long func_long() { kDebug(); return 0; }
    long long func_ll() { kDebug(); return 0; }
    unsigned long long func_ull() { kDebug(); return 0; }
    char func_char() { kDebug(); return 0; }
    signed char func_schar() { kDebug(); return 0; }
    unsigned char func_uchar() { kDebug(); return 0; }
    char *func_Pchar() { kDebug(); return 0; }
    const char *func_KPchar() { kDebug(); return 0; }
    const volatile char *func_VKPchar() { kDebug(); return 0; }
    const volatile unsigned long long * const volatile func_KVPKVull() { kDebug(); return 0; }
    const void * const volatile *func_KPKVvoid() { kDebug(); return 0; }

    QList<int> func_ai() { kDebug(); return QList<int>(); }
    QList<unsigned long long const volatile*> func_aptr() { kDebug(); return QList<unsigned long long const volatile*>(); }

    QList<Something> func_aenum() { kDebug(); return QList<Something>(); }
    QList<QList<const void *> > func_aaptr() { kDebug(); return QList<QList<const void *> >(); }

    QMap<int, Something> func_ienummap() { kDebug(); return QMap<int, Something>(); }

    template<typename T>
    T* func_template1() { kDebug(); return 0; }
    template<Something val>
    long func_template2() { kDebug(); return long(val); }

    typedef unsigned long long * ( *fptr)();
    typedef unsigned long long * (TestClass1::* pmf)();
    typedef fptr (TestClass1::* uglypmf)();
    fptr func_fptr() { kDebug(); return 0; }
    pmf func_pmf() { kDebug(); return 0; }
    uglypmf func_uglypmf(uglypmf = 0) { kDebug(); return 0; }
    QMap<QString, uglypmf> func_uglypmf2() { kDebug(); return QMap<QString, uglypmf>(); }

public:
    TestClass1()
        {
            // instantiate
            func_void();
            func_int();
            func_unsigned();
            func_long();
            func_ll();
            func_ull();
            func_char();
            func_schar();
            func_uchar();
            func_Pchar();
            func_KPchar();
            func_VKPchar();
            func_KVPKVull();
            func_KPKVvoid();
            func_ai();
            func_aptr();
            func_aenum();
            func_aaptr();
            func_ienummap();
            func_template1<TestClass1>();
            func_template2<foo>();
            func_fptr();
            func_pmf();
            func_uglypmf();
            func_uglypmf2();
        }
};

template<typename T> class TestClass2
{
    long func_long() { kDebug(); return 0; }
    template<typename S>
    T* func_template1() { kDebug(); return 0; }
    template<TestClass1::Something val>
    long func_template2() { kDebug(); return long(val); }
public:
    TestClass2()
        {
            func_long();
            func_template1<TestClass2>();
            func_template2<TestClass1::foo>();
        }
};

template<typename T, TestClass1::Something v> class TestClass3
{
    long func_long() { kDebug(); return 0; }
    template<typename S>
    S* func_template1() { kDebug(); return 0; }
    template<TestClass1::Something val>
    long func_template2() { kDebug(); return long(val); }
public:
    struct Foo { TestClass3 foo; };
    TestClass3()
        {
            func_long();
            func_template1<TestClass2<T> >();
            func_template2<TestClass1::foo>();
        }
};

class TestClass4
{
    TestClass1 c1;

    TestClass2<std::map<long, const void *> > func2()
        { kDebug(); return TestClass2<std::map<long, const void *> >(); }
    TestClass3<std::map<std::list<int>, const void *>, TestClass1::foo>::Foo func3()
        { kDebug(); return TestClass3<std::map<std::list<int>, const void *>, TestClass1::foo>::Foo(); }
public:
    TestClass4()
        {
            func2();
            func3();
            kDebug();
        }
    ~TestClass4()
        {
            kDebug();
        }
};

namespace
{
    class TestClass5
    {
    public:
        TestClass5()
        {
            kDebug();
        }
    };
    TestClass5 func5()
    {
        kDebug();
        return TestClass5();
    }
} // namespace

namespace N
{
namespace
{
    class TestClass6
    {
    public:
        TestClass6()
        {
            kDebug();
        }
    };
    TestClass6 func6()
    {
        kDebug();
        return TestClass6();
    }
} // namespace
} // namespace N

void testKDebug()
{
    QString test = "%20C this is a string";
    kDebug(180) << test;
    QByteArray cstr = test.toLatin1();
    kDebug(180) << test;
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

    kDebug() << "Printing a null QObject pointer: " << (QObject*)0;

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

#ifdef Q_CC_GNU
    bool dotest = true;
#else
    bool dotest = false;
#endif
    if (dotest) {
        kDebug() << "Testing the function names. The following should display only the base name of the functions";
        TestClass4 c4;
        func5();
        using namespace N;
        func6();
    }
}

// Concurrency testing, based on code from bug 133026
// Copyright 2006 Marcel Wiesweg <marcel.wiesweg@gmx.de>

//#define THREAD_TEST 1

#ifdef THREAD_TEST
#include <QThread>
class DebugKDebug : public QThread
{
public:
    DebugKDebug(int num) : m_num(num) {}
protected:
    virtual void run()
    {
        int count = 1000;
        while (--count) {
            kDebug() << "Test" << m_num;
            //usleep(1);
        }
    }
private:
    int m_num;
};

static void startThreads()
{
    QVector<DebugKDebug*> threads;
    threads.resize(100);
    for (int i=0; i<threads.size(); i++) {
        threads[i] = new DebugKDebug(i);
        threads[i]->start();
    }
    for (int i=0; i<threads.size(); i++) {
        threads[i]->wait();
    }
}
#endif

int main(int argc, char** argv)
{
    testKDebug();
#ifdef THREAD_TEST
    QCoreApplication app(argc, argv);
    startThreads();
#else
    Q_UNUSED(argc);
    Q_UNUSED(argv);
#endif
    return 0;
}


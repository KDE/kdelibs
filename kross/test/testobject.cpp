/***************************************************************************
 * testobject.cpp
 * This file is part of the KDE project
 * copyright (C)2004-2005 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "testobject.h"

#include <QtCore/QSize>
#include <QtCore/QPoint>
#include <QtCore/QRect>
#include <QtCore/QDate>
#include <QtCore/QTimer>
#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtGui/QBrush>

TestObject::TestObject(QObject* parent, const QString& name)
    : QObject(parent)
{
    setObjectName(name);

    qRegisterMetaType<TestObject*>("TestObject");
    //qRegisterMetaType< QList<TestObject*> >("QList<TestObject*>");

    QTimer* timer = new QTimer(this);
    timer->setObjectName("TestTimer");

    setProperty("dynamicProperty",QVariantList() << "Some String" << 99);
}

TestObject::~TestObject()
{
}

QObject* TestObject::createThread(int steps, int msecs, bool start)
{
    TestThread* t = new TestThread(this, steps, msecs);
    if( start )
        t->run();
    return t;
}

QString TestObject::name()
{
    return objectName();
}

void TestObject::func_void()
{
    //kDebug() << "TestObject::func_void";
}

void TestObject::func_void_int(int)
{
    //kDebug() << "TestObject::func_void_int i=" << i;
}

int TestObject::func_int_int(int i)
{
    //kDebug() << "TestObject::func_int_int i=" << i;
    return i;
}

bool TestObject::func_bool_bool(bool b)
{
    //kDebug() << "TestObject::func_bool_bool b=" << b;
    return b;
}

uint TestObject::func_uint_uint(uint i)
{
    //kDebug() << "TestObject::func_uint_uint i=" << i;
    return i;
}

double TestObject::func_double_double(double d)
{
    //kDebug() << "TestObject::func_double_double d=" << d;
    return d;
}

qlonglong TestObject::func_qlonglong_qlonglong(qlonglong l)
{
    //kDebug() << "TestObject::func_qlonglong_qlonglong l=" << l;
    return l;
}

qulonglong TestObject::func_qulonglong_qulonglong(qulonglong l)
{
    //kDebug() << "TestObject::func_qulonglong_qulonglong l=" << l;
    return l;
}

void TestObject::func_void_qstring_int(QString, int)
{
    //kDebug() << "TestObject::func_void_qstring_int s=" << s << " i=" << i;
}

void TestObject::func_void_qstringlist(QStringList)
{
    //kDebug() << "TestObject::func_void_qstringlist sl=" << sl.join(",");
}

QString TestObject::func_qstring_qstring_int(QString s, int i)
{
    QString r = QString("%1,%2").arg(s).arg(i);
    //kDebug() << "TestObject::func_void_qstring_int s=" << s << " i=" << i << " returnvalue=" << r;
    return r;
}

QByteArray TestObject::func_qbytearray_qbytearray(QByteArray ba)
{
    //kDebug() << "TestObject::func_qbytearray_qbytearray ba=" << ba;
    return ba;
}

QString TestObject::func_qstring_qstring(const QString& s)
{
    //kDebug() << "TestObject::func_qstring_qstring s=" << s;
    return s;
}

QStringList TestObject::func_qstringlist_qstringlist(QStringList sl)
{
    //kDebug() << "TestObject::func_qstringlist_qstringlist sl=" << sl.join(",");
    return sl;
}

QVariantList TestObject::func_qvariantlist_qvariantlist(QVariantList vl)
{
    //kDebug() << "TestObject::func_qvariantlist_qvariantlist vl.count=" << vl.count();
    return vl;
}

QVariantMap TestObject::func_qvariantmap_qvariantmap(QVariantMap vm)
{
    //kDebug() << "TestObject::func_qvariantmap_qvariantmap vm.count=" << vm.count();
    return vm;
}

QVariantList TestObject::func_qobject2qvariantlist(QObject* obj)
{
    //kDebug()<<"TestObject::func_qobject2qvariantlist obj="<<(obj ? obj->objectName() : "NULL");
    QVariant v;
    v.setValue(obj);
    return QVariantList() << v;
}

QObject* TestObject::func_qvariantlist2qobject(QVariantList list)
{
    return list.count() > 0 ? list[0].value<QObject*>() : 0;
}

QSize TestObject::func_qsize_qsize(const QSize& s) { return s; }
QSizeF TestObject::func_qsizef_qsizef(const QSizeF& s) { return s; }
QPoint TestObject::func_qpoint_qpoint(const QPoint& p) { return p; }
QPointF TestObject::func_qpointf_qpointf(const QPointF& p) { return p; }
QRect TestObject::func_qrect_qrect(const QRect& r) { return r; }
QRectF TestObject::func_qrectf_qrectf(const QRectF& r) { return r; }
QUrl TestObject::func_qurl_qurl(const QUrl& u) { return u; }
QUrl TestObject::func_qurl_kurl(const KUrl& u) { return u; }
KUrl TestObject::func_kurl_kurl(const KUrl& u) { return u; }
const KUrl& TestObject::func_kurl_qstring(const QString& u) { return KUrl(u); }

QColor TestObject::func_qcolor_qcolor(const QColor& c) { return c; }
QFont TestObject::func_qfont_qfont(const QFont& f) { return f; }
QBrush TestObject::func_qbrush_qbrush(const QBrush& b) { return b; }
QTime TestObject::func_qtime_qtime(const QTime& t) { return t; }
QDate TestObject::func_qdate_qdate(const QDate& d) { return d; }
QDateTime TestObject::func_qdatetime_qdatetime(const QDateTime& dt) { return dt; }

void TestObject::func_void_qvariant(const QVariant& v)
{
    kDebug() << "TestObject::func_void_qvariant variant.toString=" << v.toString() << " variant.type=" << v.typeName();
}

QVariant TestObject::func_qvariant_qvariant(const QVariant& v)
{
    //kDebug() << "TestObject::func_qvariant_qvariant value=" << v.toString() << " type=" << v.typeName();
    return v;
}

Kross::Object::Ptr TestObject::func_krossobject_krossobject(Kross::Object::Ptr object)
{
    //kDebug() << "TestObject::func_krossobject_krossobject";
    return object;
}

QStringList TestObject::get_krossobject_methods(Kross::Object::Ptr object)
{
    //kDebug() << "TestObject::get_krossobject_methods";
    return object->methodNames();
}

QVariant TestObject::call_krossobject_method(Kross::Object::Ptr object, const QString& methodname, const QVariantList& args)
{
    //kDebug() << "TestObject::call_krossobject_method";
    return object->callMethod(methodname, args);
}

QObject* TestObject::func_createChildTestObject(const QString& objectname)
{
    return new TestObject(this, objectname);
}

void TestObject::func_void_qobject(QObject*)
{
    //kDebug() << "TestObject::func_void_qobject " << (obj ? QString("objectName=%1 className=%2").arg(obj->objectName()).arg(obj->metaObject()->className()) : "NULL");
}

QObject* TestObject::func_qobject_qobject(QObject* obj)
{
    //kDebug() << "TestObject::func_qobject_qobject " << (obj ? QString("objectName=%1 className=%2").arg(obj->objectName()).arg(obj->metaObject()->className()) : "NULL");
    return obj;
}

QWidget* TestObject::func_qwidget_qwidget(QWidget* widget)
{
    kDebug() << "TestObject::func_qwidget_qwidget " << (widget ? QString("objectName=%1 className=%2").arg(widget->objectName()).arg(widget->metaObject()->className()) : "NULL");
    return widget;
}

void TestObject::func_void_testobject(TestObject* obj)
{
    kDebug() << "TestObject::func_void_testobject " << (obj ? QString("objectName=%1 className=%2").arg(obj->objectName()).arg(obj->metaObject()->className()) : "NULL");
}

TestObject* TestObject::func_testobject_testobject(TestObject* obj)
{
    kDebug() << "TestObject::func_testobject_testobject " << (obj ? QString("objectName=%1 className=%2").arg(obj->objectName()).arg(obj->metaObject()->className()) : "NULL");
    return obj;
}

TestObject* TestObject::func_testobject_qobject(QObject* obj)
{
    TestObject* tobj = dynamic_cast<TestObject*>(obj);
    return tobj;
}

void TestObject::func_void_testobjectlist(QList<TestObject*> l)
{
    kDebug() << "TestObject::func_void_testobjectlist " << l.count();
    foreach(TestObject* obj, l)
        kDebug() << "  " << (obj ? obj->objectName() : "NULL");
}

QList<TestObject*> TestObject::func_testobjectlist_testobjectlist(QList<TestObject*> l)
{
    kDebug() << "TestObject::func_testobjectlist_testobjectlist " << l.count();
    return l;
}

/*****************************************************************************************
 * OtherObject
 */

OtherObject* TestObject::func_otherobject(const QByteArray& name)
{
    if( OtherObject* obj = findChild<OtherObject*>(name) )
        return obj;
    return new OtherObject(this, name);
}

OtherObject* TestObject::func_otherobject_otherobject(OtherObject* obj)
{
    return obj;
}

QList<OtherObject*> TestObject::func_otherobjectlist_otherobjectlist(QList<OtherObject*> l)
{
    return l;
}

/*****************************************************************************************
 * TestThread
 */

TestThread::TestThread(TestObject* parent, int steps, int msecs)
    : QThread(parent)
    , m_testobject(parent)
    , m_steps(steps)
    , m_msecs(msecs)
{
    //kDebug() << "TestThread::TestThread()";
}

TestThread::~TestThread()
{
    //kDebug() << "TestThread::~TestThread()";
}

void TestThread::emitStepDone(int step)
{
    //kDebug() << "TestThread::emitStepDone() step=" << step;
    emit stepDone(step);
}

QVariant TestThread::callFunction(QObject* action, const QString& functionname, QVariantList arguments)
{
    Kross::Action* a = dynamic_cast<Kross::Action*>(action);
    Q_ASSERT(a);
    return a->callFunction(functionname, arguments);
}

void TestThread::run()
{
    //kDebug() << "TestThread::run()";
    for(int i = 0; i < m_steps; ++i) {
        kDebug() << "TestThread::run() 1 step=" << i << "m_steps=" << m_steps << "m_msecs=" << m_msecs;
        emit stepDone(i);
        //m_testobject->emitSignalVoid();
        msleep(m_msecs);
    }
}

#include "testobject.moc"

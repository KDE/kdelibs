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

#include <QSize>
#include <QSizeF>
#include <QPoint>
#include <QPointF>
#include <QRect>
#include <QRectF>

#include <QColor>
#include <QFont>
#include <QBrush>
#include <QDate>
#include <QTime>
#include <QDateTime>

TestObject::TestObject(QObject* parent, const QString& name)
    : QObject(parent)
{
    setObjectName(name);
    //kDebug() << "TestObject(const QString&) objectName=" << objectName() << endl;

    //connect(this, SIGNAL(testSignal()), this, SLOT(testSignalSlot()));
    //connect(this, SIGNAL(stdoutSignal(const QString&)), this, SLOT(stdoutSlot(const QString&)));
    //connect(this, SIGNAL(stderrSignal(const QString&)), this, SLOT(stderrSlot(const QString&)));

    //Action->addQObject(this);
    //Action->addSignal("stdout", this, SIGNAL(stdoutSignal(const QString&)));
    //Action->addSlot("stderr", this, SLOT(stderrSlot(const QString&)));
    //Action->addSignal("myTestSignal", this, SIGNAL(testSignal()));
    //Action->addSlot("myTestSlot", this, SLOT(testSlot()));
}

TestObject::~TestObject()
{
}

QString TestObject::name()
{
    return objectName();
}

void TestObject::func_void()
{
    //kDebug() << "TestObject::func_void" << endl;
}

void TestObject::func_void_int(int)
{
    //kDebug() << "TestObject::func_void_int i=" << i << endl;
}

int TestObject::func_int_int(int i)
{
    //kDebug() << "TestObject::func_int_int i=" << i << endl;
    return i;
}

bool TestObject::func_bool_bool(bool b)
{
    //kDebug() << "TestObject::func_bool_bool b=" << b << endl;
    return b;
}

uint TestObject::func_uint_uint(uint i)
{
    //kDebug() << "TestObject::func_uint_uint i=" << i << endl;
    return i;
}

double TestObject::func_double_double(double d)
{
    //kDebug() << "TestObject::func_double_double d=" << d << endl;
    return d;
}

qlonglong TestObject::func_qlonglong_qlonglong(qlonglong l)
{
    //kDebug() << "TestObject::func_qlonglong_qlonglong l=" << l << endl;
    return l;
}

qulonglong TestObject::func_qulonglong_qulonglong(qulonglong l)
{
    //kDebug() << "TestObject::func_qulonglong_qulonglong l=" << l << endl;
    return l;
}

void TestObject::func_void_qstring_int(QString, int)
{
    //kDebug() << "TestObject::func_void_qstring_int s=" << s << " i=" << i << endl;
}

void TestObject::func_void_qstringlist(QStringList)
{
    //kDebug() << "TestObject::func_void_qstringlist sl=" << sl.join(",") << endl;
}

QString TestObject::func_qstring_qstring_int(QString s, int i)
{
    QString r = QString("%1,%2").arg(s).arg(i);
    //kDebug() << "TestObject::func_void_qstring_int s=" << s << " i=" << i << " returnvalue=" << r << endl;
    return r;
}

QByteArray TestObject::func_qbytearray_qbytearray(QByteArray ba)
{
    //kDebug() << "TestObject::func_qbytearray_qbytearray ba=" << ba << endl;
    return ba;
}

QString TestObject::func_qstring_qstring(const QString& s)
{
    //kDebug() << "TestObject::func_qstring_qstring s=" << s << endl;
    return s;
}

QStringList TestObject::func_qstringlist_qstringlist(QStringList sl)
{
    //kDebug() << "TestObject::func_qstringlist_qstringlist sl=" << sl.join(",") << endl;
    return sl;
}

QVariantList TestObject::func_qvariantlist_qvariantlist(QVariantList vl)
{
    //kDebug() << "TestObject::func_qvariantlist_qvariantlist vl.count=" << vl.count() << endl;
    return vl;
}

QVariantMap TestObject::func_qvariantmap_qvariantmap(QVariantMap vm)
{
    //kDebug() << "TestObject::func_qvariantmap_qvariantmap vm.count=" << vm.count() << endl;
    return vm;
}

QSize TestObject::func_qsize_qsize(const QSize& s) { return s; }
QSizeF TestObject::func_qsizef_qsizef(const QSizeF& s) { return s; }
QPoint TestObject::func_qpoint_qpoint(const QPoint& p) { return p; }
QPointF TestObject::func_qpointf_qpointf(const QPointF& p) { return p; }
QRect TestObject::func_qrect_qrect(const QRect& r) { return r; }
QRectF TestObject::func_qrectf_qrectf(const QRectF& r) { return r; }

QColor TestObject::func_qcolor_qcolor(const QColor& c) { return c; }
QFont TestObject::func_qfont_qfont(const QFont& f) { return f; }
QBrush TestObject::func_qbrush_qbrush(const QBrush& b) { return b; }
QTime TestObject::func_qtime_qtime(const QTime& t) { return t; }
QDate TestObject::func_qdate_qdate(const QDate& d) { return d; }
QDateTime TestObject::func_qdatetime_qdatetime(const QDateTime& dt) { return dt; }

QVariant TestObject::func_qvariant_qvariant(const QVariant& v)
{
    //kDebug() << "TestObject::func_qvariant_qvariant value=" << v.toString() << " type=" << v.typeName() << endl;
    return v;
}

/*
void TestObject::func_void_krossobject(Kross::Object::Ptr obj)
{
    kDebug() << "TestObject::func_void_krossobject " << (obj ? QString("objectName=%1 className=%2").arg(obj->objectName()).arg(obj->metaObject()->className()) : "NULL") << endl;
}

Kross::Object::Ptr TestObject::func_krossobject_krossobject(Kross::Object::Ptr obj)
{
    kDebug() << "TestObject::func_krossobject_krossobject " << (obj ? QString("objectName=%1 className=%2").arg(obj->objectName()).arg(obj->metaObject()->className()) : "NULL") << endl;
    return obj;
}
*/

QObject* TestObject::func_createChildTestObject(const QString& objectname)
{
    return new TestObject(this, objectname);
}

void TestObject::func_void_qobject(QObject*)
{
    //kDebug() << "TestObject::func_void_qobject " << (obj ? QString("objectName=%1 className=%2").arg(obj->objectName()).arg(obj->metaObject()->className()) : "NULL") << endl;
}

QObject* TestObject::func_qobject_qobject(QObject* obj)
{
    //kDebug() << "TestObject::func_qobject_qobject " << (obj ? QString("objectName=%1 className=%2").arg(obj->objectName()).arg(obj->metaObject()->className()) : "NULL") << endl;
    return obj;
}

void TestObject::func_void_testobject(TestObject*)
{
    //kDebug() << "TestObject::func_void_testobject " << (obj ? QString("objectName=%1 className=%2").arg(obj->objectName()).arg(obj->metaObject()->className()) : "NULL") << endl;
}

TestObject* TestObject::func_testobject_testobject(TestObject* obj)
{
    //kDebug() << "TestObject::func_testobject_testobject " << (obj ? QString("objectName=%1 className=%2").arg(obj->objectName()).arg(obj->metaObject()->className()) : "NULL") << endl;
    return obj;
}

#include "testobject.moc"

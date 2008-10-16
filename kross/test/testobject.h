/***************************************************************************
 * testobject.h
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

#ifndef KROSS_TEST_TESTOBJECT_H
#define KROSS_TEST_TESTOBJECT_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtCore/QMetaType>
#include <QtCore/QUrl>
#include <QtCore/QThread>

class QSize;
class QSizeF;
class QPoint;
class QPointF;
class QRect;
class QRectF;

class QColor;
class QFont;
class QBrush;
class QDate;
class QTime;
class QDateTime;

#include <kurl.h>
#include <kdebug.h>

#include "../core/action.h"
#include "../core/object.h"
#include "../core/wrapperinterface.h"

class OtherObject;

/// \internal class to test functionality within krosstest
class TestObject : public QObject
{
        Q_OBJECT

        Q_PROPERTY(bool boolProperty READ boolProperty WRITE setBoolProperty)
        Q_PROPERTY(int intProperty READ intProperty WRITE setIntProperty)
        Q_PROPERTY(double doubleProperty READ doubleProperty WRITE setDoubleProperty)
        Q_PROPERTY(QString stringProperty READ stringProperty WRITE setStringProperty)
        Q_PROPERTY(QStringList stringListProperty READ stringListProperty WRITE setStringListProperty)
        Q_PROPERTY(QVariantList listProperty READ listProperty WRITE setListProperty)

        Q_ENUMS(TestEnum)

    public:
        explicit TestObject(QObject* parent = 0, const QString& name = QString());
        //TestObject(const TestObject&) {}
        virtual ~TestObject();

        enum TestEnum { TESTENUM1 = 1, TESTENUM2 = 2, TESTENUM3 = 4, TESTENUM4 = 8 };

    private:

        bool m_boolproperty;
        bool boolProperty() const { return m_boolproperty; }
        void setBoolProperty(bool prop) { m_boolproperty = prop; }

        int m_intproperty;
        int intProperty() const { return m_intproperty; }
        void setIntProperty(int prop) { m_intproperty = prop; }

        double m_doubleproperty;
        double doubleProperty() const { return m_doubleproperty; }
        void setDoubleProperty(double prop) { m_doubleproperty = prop; }

        QString m_stringproperty;
        QString stringProperty() const { return m_stringproperty; }
        void setStringProperty(QString prop) { m_stringproperty = prop; }

        QStringList m_stringlistproperty;
        QStringList stringListProperty() const { return m_stringlistproperty; }
        void setStringListProperty(QStringList prop) { m_stringlistproperty = prop; }

        QVariantList m_listproperty;
        QVariantList listProperty() const { return m_listproperty; }
        void setListProperty(QVariantList prop) { m_listproperty = prop; }

    Q_SIGNALS:
        void signalVoid();
        void signalBool(bool);
        void signalInt(int);
        void signalString(const QString&);
        void signalObject(QObject* obj);

    public Q_SLOTS:

        QObject* myself() { return this; }
        TestObject* myself2() { return this; }
        QList<TestObject*> myself3() { return QList<TestObject*>() << this; }

        // return a TestThread instance.
        QObject* createThread(int steps, int msecs, bool start = false);

        // emit some signals
        void emitSignalVoid() { emit signalVoid(); }
        void emitSignalBool(bool b) { emit signalBool(b); }
        void emitSignalInt(int i) { emit signalInt(i); }
        void emitSignalString(const QString& s) { emit signalString(s); }
        void emitSignalObject(QObject* obj) { emit signalObject(obj); }

        // return the objectname
        QString name();

        // test the enumerator
        //TestEnum testEnum(TestEnum e) const { return e; }

        // to test basic datatypes
        int func_int_int(int);
        bool func_bool_bool(bool);
        uint func_uint_uint(uint);
        double func_double_double(double);
        qlonglong func_qlonglong_qlonglong(qlonglong);
        qulonglong func_qulonglong_qulonglong(qulonglong);
        QByteArray func_qbytearray_qbytearray(QByteArray);
        QString func_qstring_qstring(const QString&);
        QStringList func_qstringlist_qstringlist(QStringList);
        QVariantList func_qvariantlist_qvariantlist(QVariantList);
        QVariantMap func_qvariantmap_qvariantmap(QVariantMap);

        QVariantList func_qobject2qvariantlist(QObject* obj);
        QObject* func_qvariantlist2qobject(QVariantList list);

        QSize func_qsize_qsize(const QSize&);
        QSizeF func_qsizef_qsizef(const QSizeF&);
        QPoint func_qpoint_qpoint(const QPoint&);
        QPointF func_qpointf_qpointf(const QPointF&);
        QRect func_qrect_qrect(const QRect&);
        QRectF func_qrectf_qrectf(const QRectF&);
        QUrl func_qurl_qurl(const QUrl&);
        QUrl func_qurl_kurl(const KUrl&);
        KUrl func_kurl_kurl(const KUrl&);

        // following is returned by reference which is a big nono
        // cause it may lead to crashes. So, the backends should
        // set such things to None/nil/NULL/etc.
        const KUrl& func_kurl_qstring(const QString&);

        QColor func_qcolor_qcolor(const QColor&);
        QFont func_qfont_qfont(const QFont&);
        QBrush func_qbrush_qbrush(const QBrush&);
        QTime func_qtime_qtime(const QTime&);
        QDate func_qdate_qdate(const QDate&);
        QDateTime func_qdatetime_qdatetime(const QDateTime&);
        void func_void_qvariant(const QVariant&);
        QVariant func_qvariant_qvariant(const QVariant&);

        // for misc tests
        void func_void();
        void func_void_int(int);
        void func_void_qstring_int(QString,int);
        void func_void_qstringlist(QStringList);
        QString func_qstring_qstring_int(QString,int);

        // Kross::Object
        Kross::Object::Ptr func_krossobject_krossobject(Kross::Object::Ptr object);
        QStringList get_krossobject_methods(Kross::Object::Ptr object);
        QVariant call_krossobject_method(Kross::Object::Ptr object, const QString& methodname, const QVariantList& args = QVariantList());

        // QObject
        QObject* func_createChildTestObject(const QString& objectname);
        void func_void_qobject(QObject*);
        QObject* func_qobject_qobject(QObject*);

        // QWidget
        QWidget* func_qwidget_qwidget(QWidget*);

        // TestObject
        void func_void_testobject(TestObject*);
        TestObject* func_testobject_testobject(TestObject*);
        TestObject* func_testobject_qobject(QObject*);
        void func_void_testobjectlist(QList<TestObject*>);
        QList<TestObject*> func_testobjectlist_testobjectlist(QList<TestObject*>);

        // OtherObject
        OtherObject* func_otherobject(const QByteArray& name);
        OtherObject* func_otherobject_otherobject(OtherObject*);
        QList<OtherObject*> func_otherobjectlist_otherobjectlist(QList<OtherObject*>);
};

/// \internal class used in TestObject to test functionality within krosstest
class OtherObject : public QObject
{
        Q_OBJECT
    public:
        explicit OtherObject(TestObject* testobj, const QByteArray& name) : QObject(testobj) { setObjectName(name); }
    public Q_SLOTS:
        QObject* testObject() const { return parent(); }
};

/// \internal class used in a handler within krosstest to provide a OtherObject wrapper on demand
class OtherObjectWrapper : public QObject, public Kross::WrapperInterface
{
        Q_OBJECT
    public:
        OtherObjectWrapper(QObject* obj) : QObject(obj) {
            Q_ASSERT(obj);
            setObjectName(QString("%1_wrapper").arg(obj->objectName()).toLatin1());
        }
        void* wrappedObject() const { return parent(); }
    public Q_SLOTS:
        QObject* parentObject() const { return parent(); }
};

/**
* \internal class to test threading functionality within krosstest.
*
* Following python code does provide a sample how this class
* may used to test the threading functionality.
* \code
* import time, TestObject1
* # this function will be called by our thread
* def myFunction(step):
*     print "myFunction step=%i" % step
* # create a thread that iterates 15 times and waits 200
* # milliseconds on each iteration, 20*200=2 seconds
* mythread = TestObject1.createThread(15,200)
* # connect the stepDone signal our thread provides to
* # our own python function above
* mythread.connect("stepDone(int)",myFunction)
* # start the thread
* mythread.start()
* # let this python thread sleep for a second
* time.sleep(1)
* # ask our action to call our myFunction function
* self.callFunction("myFunction", [123])
* # ask our thread to emit the stepDone signal which in
* # turn should call our myFunction function.
* mythread.emitStepDone(456)
* # same as above but we emit the signal by our own
* # rather than going through a helper-function.
* mythread.stepDone(789)
* # now let's ask our thread to ask our action to execute
* # our function to have one complete roundtrip :)
* mythread.callFunction(self, "myFunction", [987])
* # sleep again one second
* time.sleep(1)
* \endcode
*
* Following sample code does the same as the code above but
* uses the Ruby scripting language.
* \code
* require 'TestObject1'
* def myFunction(step)
*     puts "myFunction step=" + step.to_s
* end
* mythread = TestObject1.createThread(10,200)
* mythread.connect("stepDone(int)", method("myFunction"))
* mythread.start()
* sleep(1)
* self.action().callFunction("myFunction", [123])
* mythread.emitStepDone(456)
* mythread.stepDone(789)
* mythread.callFunction(self.action(), "myFunction", [987])
* sleep(1)
* \endcode
*/
class TestThread : public QThread
{
        Q_OBJECT
    public:
        explicit TestThread(TestObject* parent, int steps, int msecs);
        virtual ~TestThread();
        virtual void run();
    public Q_SLOTS:
        void emitStepDone(int step);
        QVariant callFunction(QObject* action, const QString& functionname, QVariantList arguments);
    Q_SIGNALS:
        void stepDone(int step);
    private:
        TestObject* m_testobject;
        int m_steps, m_msecs;
};

//Q_DECLARE_METATYPE( TestObject* )
//Q_DECLARE_METATYPE( TestObject )

#endif

/***************************************************************************
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
*/
#include "QtTest/private/qsignaldumper_p.h"

#include <QtCore/qlist.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qobject.h>
#include <QtCore/qvariant.h>

#include "QtTest/private/qtestlog_p.h"
namespace QtTest
{

inline static void qPrintMessage(const QByteArray &ba)
{
    QtTestLog::info(ba.constData(), 0, 0);
}

Q_GLOBAL_STATIC(QList<QByteArray>, ignoreClasses)
static int iLevel = 0;
static int ignoreLevel = 0;
enum { IndentSpacesCount = 4 };

static QByteArray memberName(const QMetaMethod &member)
{
    QByteArray ba = member.signature();
    return ba.left(ba.indexOf('('));
}

static void qSignalDumperCallback(QObject *caller, int method_index, void **argv)
{
    Q_ASSERT(caller); Q_ASSERT(argv); Q_UNUSED(argv);
    const QMetaObject *mo = caller->metaObject();
    Q_ASSERT(mo);
    QMetaMethod member = mo->method(method_index);
    Q_ASSERT(member.signature());

    if (QtTest::ignoreClasses()->contains(mo->className())) {
        ++QtTest::ignoreLevel;
        return;
    }

    QByteArray str;
    str.fill(' ', QtTest::iLevel++ * QtTest::IndentSpacesCount);
    str += "Signal: ";
    str += mo->className();
    str += "(";
    str += caller->objectName().toLocal8Bit();
    str += ") ";
    str += QtTest::memberName(member);
    str += " (";

    QList<QByteArray> args = member.parameterTypes();
    for (int i = 0; i < args.count(); ++i) {
        str += args.at(i);
        int typeId = QMetaType::type(args.at(i).constData());
        if (args.at(i).endsWith("*")) {
            str.append("(0x").append(QByteArray::number(reinterpret_cast<quint64>(*reinterpret_cast<void **>(argv[i + 1])), 16)).append(")");
        } else if (typeId != QMetaType::Void) {
            str.append("(").append(QVariant(typeId, argv[i + 1]).toString().toLocal8Bit()).append(
            ")");
        }
        str.append(", ");
    }
    if (str.endsWith(", "))
        str.chop(2);
    str.append(")");
    qPrintMessage(str);
}

static void qSignalDumperCallbackSlot(QObject *caller, int method_index, void **argv)
{
    Q_ASSERT(caller); Q_ASSERT(argv); Q_UNUSED(argv);
    const QMetaObject *mo = caller->metaObject();
    Q_ASSERT(mo);
    QMetaMethod member = mo->method(method_index);
    if (!member.signature())
        return;

    if (QtTest::ignoreLevel || QtTest::ignoreClasses()->contains(mo->className()))
        return;

    QByteArray str;
    str.fill(' ', QtTest::iLevel * QtTest::IndentSpacesCount);
    str += "Slot: ";
    str += mo->className();
    str += "(";
    str += caller->objectName().toLocal8Bit();
    str += ") ";
    str += member.signature();
    qPrintMessage(str);
}

static void qSignalDumperCallbackEndSignal(QObject *caller, int /*method_index*/)
{
    Q_ASSERT(caller); Q_ASSERT(caller->metaObject());
    if (QtTest::ignoreClasses()->contains(caller->metaObject()->className())) {
        --QtTest::ignoreLevel;
        Q_ASSERT(QtTest::ignoreLevel >= 0);
        return;
    }
    --QtTest::iLevel;
    Q_ASSERT(QtTest::iLevel >= 0);
}

}

// this struct is copied from qobject_p.h to prevent us
// from including private Qt headers.
struct QSignalSpyCallbackSet
{
    typedef void (*BeginCallback)(QObject *caller, int method_index, void **argv);
    typedef void (*EndCallback)(QObject *caller, int method_index);
    BeginCallback signal_begin_callback,
                  slot_begin_callback;
    EndCallback signal_end_callback,
                slot_end_callback;
};
extern void Q_CORE_EXPORT qt_register_signal_spy_callbacks(const QSignalSpyCallbackSet &);

void QSignalDumper::startDump()
{
    static QSignalSpyCallbackSet set = { QtTest::qSignalDumperCallback,
        QtTest::qSignalDumperCallbackSlot, QtTest::qSignalDumperCallbackEndSignal, 0 };
    qt_register_signal_spy_callbacks(set);
}

void QSignalDumper::endDump()
{
    static QSignalSpyCallbackSet nset = { 0, 0, 0 ,0 };
    qt_register_signal_spy_callbacks(nset);
}

void QSignalDumper::ignoreClass(const QByteArray &klass)
{
    QtTest::ignoreClasses()->append(klass);
}

void QSignalDumper::clearIgnoredClasses()
{
    QtTest::ignoreClasses()->clear();
}


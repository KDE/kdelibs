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
/****************************************************************************
 **
 ** Definition of QSignalSpy class.
 **
 ** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
 **
 ** This file is part of the QtTest library.
 ** EDITIONS: NONE
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/

#ifndef QSIGNALSPY_H
#define QSIGNALSPY_H

#include <QtCore/qglobal.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qlist.h>
#include <QtCore/qobject.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qvariant.h>

class QSignalSpy: public QObject, public QList<QList<QVariant> >
{
public:
    QSignalSpy(QObject *obj, const char *signal)
    {
#ifdef Q_CC_BOR
        const int memberOffset = QObject::staticMetaObject.methodCount();
#else
        static const int memberOffset = QObject::staticMetaObject.methodCount();
#endif
        Q_ASSERT(obj);
        Q_ASSERT(signal);

        if (signal[0] - '0' != QSIGNAL_CODE) {
            qWarning("QSignalSpy: Not a valid signal, use the SIGNAL macro");
            return;
        }

        QByteArray ba = QMetaObject::normalizedSignature(signal + 1);
        const QMetaObject *mo = obj->metaObject();
        int sigIndex = mo->indexOfMethod(ba.constData());
        if (sigIndex < 0) {
            qWarning("QSignalSpy: No such signal: '%s'", ba.constData());
            return;
        }

        if (!QMetaObject::connect(obj, sigIndex, this, memberOffset,
                    Qt::DirectConnection, 0)) {
            qWarning("QSignalSpy: QMetaObject::connect returned false. Unable to connect.");
            return;
        }
        sig = ba;
        initArgs(mo->method(sigIndex));
    }

    inline bool isValid() const { return !sig.isEmpty(); }
    inline QByteArray signal() const { return sig; }


    int qt_metacall(QMetaObject::Call call, int id, void **a)
    {
        id = QObject::qt_metacall(call, id, a);
        if (id < 0)
            return id;

        if (call == QMetaObject::InvokeMetaMethod) {
            if (id == 0) {
                appendArgs(a);
            }
            --id;
        }
        return id;
    }

private:
    void initArgs(const QMetaMethod &member)
    {
        QList<QByteArray> params = member.parameterTypes();
        for (int i = 0; i < params.count(); ++i) {
            int tp = QMetaType::type(params.at(i).constData());
            if (tp == QMetaType::Void)
                qWarning("Don't know how to handle '%s', use qRegisterMetaType to register it.",
                         params.at(i).constData());
            args << tp;
        }
    }

    void appendArgs(void **a)
    {
        QList<QVariant> list;
        for (int i = 0; i < args.count(); ++i) {
            bool handled = false;
            switch (static_cast<QMetaType::Type>(args.at(i))) {
            case QMetaType::Void:
                list << QVariant();
                handled = true;
                break;
            case QMetaType::VoidStar:
                list << QVariant(QVariant::UserType, *(void **)a[i+1]);
                handled = true;
                break;
            case QMetaType::Int:
            case QMetaType::UInt:
            case QMetaType::Bool:
            case QMetaType::Double:
            case QMetaType::QByteArray:
            case QMetaType::QString:
            case QMetaType::QObjectStar:
            case QMetaType::QWidgetStar:
            case QMetaType::Long:
            case QMetaType::Short:
            case QMetaType::Char:
            case QMetaType::ULong:
            case QMetaType::UShort:
            case QMetaType::UChar:
            case QMetaType::Float:
            case QMetaType::QChar:
                list << QVariant(args.at(i), a[i+1]);
                handled = true;
                break;
            case QMetaType::User:
                break;
            // no default statement so that we get warnings for unhandled types
            }
            if (!handled) {
                Q_ASSERT(args.at(i) >= QMetaType::User);
                int tp = QVariant::nameToType(QMetaType::typeName(args.at(i)));
                Q_ASSERT(tp != QVariant::Invalid);
                list << QVariant((tp == QVariant::UserType ? args.at(i) : tp), a[i+1]);
            }
        }
        append(list);
    }

    // the full, normalized signal name
    QByteArray sig;
    // holds the QMetaType types for the argument list of the signal
    QList<int> args;
};

#endif


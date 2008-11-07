/*
    This file is part of the KDE libraries

    Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEDGLOBALACCELINTERFACE_H
#define KDEDGLOBALACCELINTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(QList<QStringList>)

/*
 * Proxy class for interface org.kde.KdedGlobalAccel
 */
class OrgKdeKdedGlobalAccelInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.kde.KdedGlobalAccel"; }

public:
    OrgKdeKdedGlobalAccelInterface(const QString &service, const QString &path,
                                   const QDBusConnection &connection,
                                   QObject *parent = 0)
     : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
    {
        qDBusRegisterMetaType<QList<int> >();
        qDBusRegisterMetaType<QList<QStringList> >();
    }

    ~OrgKdeKdedGlobalAccelInterface() {}

public Q_SLOTS: // METHODS
    QDBusReply<QStringList> action(int key)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(key);
        return callWithArgumentList(QDBus::Block, QLatin1String("action"),
                                    argumentList);
    }

    QDBusReply<QList<int> > shortcut(const QStringList &actionId)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(actionId);
        return callWithArgumentList(QDBus::Block, QLatin1String("shortcut"),
                                    argumentList);
    }

    QDBusReply<QList<int> > setShortcut(const QStringList &actionId,
                                        const QList<int> &keys, uint flags)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(actionId) << qVariantFromValue(keys)
                     << qVariantFromValue(flags);
        return callWithArgumentList(QDBus::Block, QLatin1String("setShortcut"),
                                    argumentList);
    }

    void setForeignShortcut(const QStringList &actionId,
                                      const QList<int> &keys)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(actionId) << qVariantFromValue(keys);
        callWithArgumentList(QDBus::Block, QLatin1String("setForeignShortcut"),
                             argumentList);
    }

    void setInactive(const QStringList &actionId)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(actionId);
        callWithArgumentList(QDBus::Block, QLatin1String("setInactive"),
                             argumentList);
    }

    void doRegister(const QStringList &actionId)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(actionId);
        callWithArgumentList(QDBus::Block, QLatin1String("doRegister"),
                             argumentList);
    }

    void unRegister(const QStringList &actionId)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(actionId);
        callWithArgumentList(QDBus::Block, QLatin1String("unRegister"),
                             argumentList);
    }

    QDBusReply<QList<QStringList> > allMainComponents()
    {
        QList<QVariant> argumentList;
        // argumentList << qVariantFromValue(void); :)
        return callWithArgumentList(QDBus::Block, QLatin1String("allMainComponents"),
                                    argumentList);
    }


    QDBusReply<QList<QStringList> > allActionsForComponent(const QStringList &actionId)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(actionId);
        return callWithArgumentList(QDBus::Block, QLatin1String("allActionsForComponent"),
                                    argumentList);
    }
    
//The signals part seems to work by pure black magic, i.e. clever hacks
//with the Qt meta-object / signals & slots system. Maybe.
Q_SIGNALS: // SIGNALS
    void invokeAction(const QStringList &actionId, qlonglong timestamp);
    void yourShortcutGotChanged(const QStringList &actionId,
                                const QList<int> &keys);
};

namespace org {
  namespace kde {
    typedef ::OrgKdeKdedGlobalAccelInterface KdedGlobalAccelInterface;
  }
}

#endif //KDEDGLOBALACCELINTERFACE_H

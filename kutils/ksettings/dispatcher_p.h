/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef DISPATCHER_P_H
#define DISPATCHER_P_H

#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtCore/QMap>
#include <QtCore/QObject>

#include <kcomponentdata.h>

namespace KSettings
{
namespace Dispatcher
{

class ComponentInfo
{
public:
    KComponentData componentData;
    typedef QPair<QObject*, const char*> Slot;
    QList<Slot> slotList;
    int count;

    ComponentInfo() : count(0) {}
};

class DispatcherPrivate : public QObject
{
    Q_OBJECT
    public:
        QMap<QString, ComponentInfo> m_componentInfo;
        QMap<QObject *, QString> m_componentName;

    public Q_SLOTS:
        void unregisterComponent(QObject *);
};

} // namespace Dispatcher
} // namespace KSettings
#endif // DISPATCHER_P_H

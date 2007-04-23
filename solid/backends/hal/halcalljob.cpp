/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#include "halcalljob.h"

#include <QTimer>

HalCallJob::HalCallJob(const QDBusConnection &connection, const QString &udi,
                        const QString &interface, const QString &methodName,
                        const QList<QVariant> &parameters)
    : KJob(), m_connection(connection), m_udi(udi),
      m_iface(interface), m_method(methodName),
      m_params(parameters)
{
}

HalCallJob::~HalCallJob()
{

}

void HalCallJob::start()
{
    QTimer::singleShot(0, this, SLOT(doStart()));
}

void HalCallJob::kill(bool /*quietly */)
{

}

void HalCallJob::doStart()
{
    QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.Hal", m_udi,
                                                       m_iface, m_method);

    foreach (QVariant param, m_params) {
        msg << param;
    }

    if (!m_connection.callWithCallback(msg, this,
                                       SLOT(callReply(const QDBusMessage &)),
                                       SLOT(callError(const QDBusError &))))
    {
        setError(1);
        setErrorText(m_connection.lastError().name()+": "+m_connection.lastError().message());
        emitResult();
    }
}

void HalCallJob::callReply(const QDBusMessage &reply)
{
    setError(0);
    emitResult();
}

void HalCallJob::callError(const QDBusError &error)
{
    setError(1);
    setErrorText(error.name()+": "+error.message());
    emitResult();
}


#include "halcalljob.moc"

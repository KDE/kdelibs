/*  This file is part of the KDE project
    Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org>

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

#include "backendcapabilities.h"
#include "factory.h"
#include "objectdescription.h"
#include <QList>
#include <QSet>
#include <QStringList>
#include <kservicetypetrader.h>
#include <kmimetype.h>
#include "phonondefs_p.h"
#include "backendinterface.h"
#include <kglobal.h>
#include "backendcapabilities_p.h"

K_GLOBAL_STATIC(Phonon::BackendCapabilitiesPrivate, globalBCPrivate)

namespace Phonon
{

BackendCapabilities::Notifier *BackendCapabilities::notifier()
{
    return globalBCPrivate;
}

#define SUPPORTS(foo) \
bool BackendCapabilities::supports ## foo() \
{ \
    QObject *m_backendObject = Factory::backend(); \
    if (!m_backendObject) \
        return false; \
    bool ret; \
    pBACKEND_GET(bool, ret, "supports"#foo); \
    return ret; \
}

SUPPORTS(Video)
SUPPORTS(OSD)
SUPPORTS(Subtitles)

QStringList BackendCapabilities::availableMimeTypes()
{
    QObject *m_backendObject = Factory::backend(true);
    if (m_backendObject)
    {
        QStringList ret;
        pBACKEND_GET(QStringList, ret, "availableMimeTypes");
        return ret;
    }
    else
    {
        const KService::List offers = KServiceTypeTrader::self()->query("PhononBackend",
                "Type == 'Service' and [X-KDE-PhononBackendInfo-InterfaceVersion] == 1");
        if (!offers.isEmpty()) {
            QStringList mimeTypes = offers.first()->serviceTypes();
            mimeTypes.removeAll("PhononBackend");
            return mimeTypes;
        }
        return QStringList();
    }
}

bool BackendCapabilities::isMimeTypeAvailable(const QString &mimeType)
{
    QObject *m_backendObject = Factory::backend(false);
    if (m_backendObject)
    {
        QStringList ret;
        pBACKEND_GET(QStringList, ret, "availableMimeTypes");
        return ret.contains(mimeType);
    }
    else
    {
        const KService::List offers = KServiceTypeTrader::self()->query("PhononBackend",
                "Type == 'Service' and [X-KDE-PhononBackendInfo-InterfaceVersion] == 1");
        if (!offers.isEmpty())
            return offers.first()->hasMimeType(KMimeType::mimeType(mimeType).data());
        return false;
    }
}

#define availableDevicesImpl(T) \
QList<T> BackendCapabilities::available ## T ## s() \
{ \
    BackendInterface *backendIface = qobject_cast<BackendInterface *>(Factory::backend()); \
    QList<T> ret; \
    if (backendIface) { \
        QSet<int> deviceIndexes = backendIface->objectDescriptionIndexes(Phonon::T ## Type); \
        foreach (int i, deviceIndexes) \
            ret.append(T::fromIndex(i)); \
    } \
    return ret; \
}

#define availableDevicesImpl2(T) \
QList<T ## Description> BackendCapabilities::available ## T ## s() \
{ \
    BackendInterface *backendIface = qobject_cast<BackendInterface *>(Factory::backend()); \
    QList<T ## Description> ret; \
    if (backendIface) { \
        QSet<int> deviceIndexes = backendIface->objectDescriptionIndexes(Phonon::T ## Type); \
        foreach (int i, deviceIndexes) \
            ret.append(T ## Description::fromIndex(i)); \
    } \
    return ret; \
}
availableDevicesImpl(AudioOutputDevice)
availableDevicesImpl(AudioCaptureDevice)
availableDevicesImpl(VideoOutputDevice)
availableDevicesImpl(VideoCaptureDevice)
availableDevicesImpl2(Visualization)
availableDevicesImpl2(AudioCodec)
availableDevicesImpl2(VideoCodec)
availableDevicesImpl2(ContainerFormat)
availableDevicesImpl2(AudioEffect)
availableDevicesImpl2(VideoEffect)

} // namespace Phonon

#include "backendcapabilities.moc"
#include "backendcapabilities_p.moc"
// vim: sw=4 ts=4

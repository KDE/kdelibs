/*  This file is part of the KDE project
    Copyright (C) 2008 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) version 3.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "factory.h"
#include "objectdescription.h"
#include "../factory.h"
#include "../globalstatic_p.h"
#include "../backendinterface.h"
#include "backendinterface.h"
#include <QtCore/QtDebug>

namespace Phonon
{
namespace Experimental
{

class FactoryPrivate : public Phonon::Experimental::Factory::Sender
{
    public:
        FactoryPrivate();
        ~FactoryPrivate();
        //QPointer<QObject> m_backendObject;

    private Q_SLOTS:
        void objectDescriptionChanged(ObjectDescriptionType);
};

PHONON_GLOBAL_STATIC(Phonon::Experimental::FactoryPrivate, globalFactory)

FactoryPrivate::FactoryPrivate()
{
    QObject *backendObj = Phonon::Factory::backend();
    Q_ASSERT(backendObj);
    //QMetaObject::invokeMethod(backendObj, "experimentalBackend", Qt::DirectConnection,
            //Q_RETURN_ARG(QObject *, m_backendObject));
    //if (!m_backendObject) {
        //qDebug() << "The backend does not support Phonon::Experimental";
        //return;
    //}
    connect(backendObj, SIGNAL(objectDescriptionChanged(ObjectDescriptionType)),
            SLOT(objectDescriptionChanged(ObjectDescriptionType)));
}

FactoryPrivate::~FactoryPrivate()
{
}

void FactoryPrivate::objectDescriptionChanged(ObjectDescriptionType type)
{
    qDebug() << Q_FUNC_INFO << type;
    switch (type) {
    case VideoCaptureDeviceType:
        emit availableVideoCaptureDevicesChanged();
        break;
    default:
        break;
    }
}

Factory::Sender *Factory::sender()
{
    return globalFactory;
}

QObject *Factory::createVideoDataOutput(QObject *parent)
{
    Phonon::BackendInterface *b = qobject_cast<Phonon::BackendInterface *>(Phonon::Factory::backend());
    if (b) {
        return Phonon::Factory::registerQObject(b->createObject(
                    static_cast<Phonon::BackendInterface::Class>(Phonon::Experimental::BackendInterface::VideoDataOutputClass),
                    parent));
    }
    return 0;
}

} // namespace Experimental
} // namespace Phonon

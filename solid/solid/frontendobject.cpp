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

#include "frontendobject.h"

namespace Solid
{
    class FrontendObject::Private
    {
    public:
        QObject *backendObject;
    };
}

Solid::FrontendObject::FrontendObject( QObject *parent )
    : QObject( parent ), d( new Private )
{
    d->backendObject = 0;
}

Solid::FrontendObject::~FrontendObject()
{
    delete d;
}

bool Solid::FrontendObject::isValid() const
{
    return d->backendObject!=0;
}

QObject *Solid::FrontendObject::backendObject() const
{
    return d->backendObject;
}

void Solid::FrontendObject::setBackendObject( QObject *backendObject )
{
    if ( d->backendObject )
    {
        disconnect( d->backendObject );
        d->backendObject->disconnect( this );
    }

    d->backendObject = backendObject;

    if ( d->backendObject )
    {
        connect( backendObject, SIGNAL( destroyed( QObject* ) ),
                 this, SLOT( slotDestroyed( QObject* ) ) );
    }
}

void Solid::FrontendObject::slotDestroyed( QObject *object )
{
    if ( d->backendObject == object )
    {
        d->backendObject = 0;
    }
}

#include "frontendobject.moc"

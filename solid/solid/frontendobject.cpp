/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>

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
#include "frontendobject_p.h"

Solid::FrontendObject::FrontendObject( QObject *parent )
    : QObject( parent ), d_ptr(new FrontendObjectPrivate)
{
    Q_D(FrontendObject);

    d->backendObject = 0;
}

Solid::FrontendObject::FrontendObject( FrontendObjectPrivate &dd, QObject *parent )
    : QObject(parent), d_ptr(&dd)
{
    Q_D(FrontendObject);

    d->backendObject = 0;
}

Solid::FrontendObject::~FrontendObject()
{
    Q_D(FrontendObject);

    delete d;
    d_ptr = 0;
}

bool Solid::FrontendObject::isValid() const
{
    Q_D(const FrontendObject);

    return d->backendObject!=0;
}

QObject *Solid::FrontendObject::backendObject() const
{
    Q_D(const FrontendObject);

    return d->backendObject;
}

void Solid::FrontendObject::setBackendObject( QObject *backendObject )
{
    Q_D(FrontendObject);

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
    Q_D(FrontendObject);

    if ( d->backendObject == object )
    {
        d->backendObject = 0;
    }
}

#include "frontendobject.moc"

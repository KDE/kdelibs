/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Valentin Rusu <kde@rusu.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ksecretsserviceitem.h"
#include "ksecretsserviceitem_p.h"
#include "ksecretsserviceitemjobs.h"
#include "dbusbackend.h"
#include "item_interface.h"

#include <QDateTime>

using namespace KSecretsService;

SecretItem::SecretItem() :
    d( new SecretItemPrivate() )
{
}

SecretItem::SecretItem( const QSharedDataPointer< SecretItemPrivate > & thatPrivate ) :
    d( thatPrivate )
{
}

SecretItem::SecretItem( SecretItemPrivate* sip ) :
    d( sip )
{
}

SecretItem::~SecretItem()
{
}

SecretItemDeleteJob * SecretItem::deleteItem( const WId &promptParentWindowId )
{
    return new SecretItemDeleteJob( this, promptParentWindowId );
}

GetSecretItemSecretJob* SecretItem::getSecret() const
{
    return new GetSecretItemSecretJob( const_cast< SecretItem*>( this ) );
}


SetSecretItemSecretJob* SecretItem::setSecret(const Secret& secret)
{
    return new SetSecretItemSecretJob( this, secret );
}

ReadItemPropertyJob * SecretItem::attributes() const
{
    return new ReadItemPropertyJob( const_cast<SecretItem*>(this), "Attributes" );
}

WriteItemPropertyJob * SecretItem::setAttributes(const QMap< QString, QString >& attributes)
{
    QVariant value;
    value.setValue<QMap<QString, QString > >(attributes);
    return new WriteItemPropertyJob( const_cast<SecretItem*>(this), "Attributes", value );
}

ReadItemPropertyJob * SecretItem::isLocked() const
{
    return new ReadItemPropertyJob( const_cast<SecretItem*>(this), "Locked" );
}

ReadItemPropertyJob * SecretItem::label() const
{
    return new ReadItemPropertyJob( const_cast<SecretItem*>(this), "Label" );
}

ReadItemPropertyJob * SecretItem::createdTime() const
{
    return new ReadItemPropertyJob( const_cast<SecretItem*>(this), "Created" );
}

ReadItemPropertyJob * SecretItem::modifiedTime() const
{
    return new ReadItemPropertyJob( const_cast<SecretItem*>(this), "Modified" );
}

WriteItemPropertyJob * SecretItem::setLabel(const QString& label)
{
    return new WriteItemPropertyJob( this, "Label", QVariant( label ) );
}

SecretItemPrivate::SecretItemPrivate() :
    itemIf(0)
{
}

SecretItemPrivate::SecretItemPrivate( const QDBusObjectPath &dbusPath ) :
    itemIf(0)
{
    itemIf = DBusSession::createItem( dbusPath );
}

SecretItemPrivate::SecretItemPrivate( const SecretItemPrivate &that ) :
    QSharedData( that ),
    itemIf( that.itemIf )
{
}

bool SecretItemPrivate::isValid() const
{
    return itemIf && itemIf->isValid();
}


#include "ksecretsserviceitem.moc"

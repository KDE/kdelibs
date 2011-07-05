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

#ifndef KSECRETSITEM_H
#define KSECRETSITEM_H

#include "ksecretsservicesecret.h"
#include "ksecretsserviceitemjobs.h"

#include <kjob.h>


namespace KSecretsService {

typedef QMap< QString, QString > QStringStringMap;

class SecretItemPrivate;
   
/**
 */
class SecretItem {
protected:
    SecretItem();
public:
    SecretItem( SecretItemPrivate * );
    
    /**
     */
    KJob * deleteItem();

    /**
     * Read the data held by the SecretItem
     */
    GetSecretJob * getSecret() const;

    /**
     * Modify the item's stored data
     */
    KJob* setSecret( const Secret &secret );

    /**
     * FIXME: This methods accesses a dbus property. should it be asynchronous ?
     */
    QMap< QString, QString> attributes() const;
    
    /**
     * FIXME: This methods accesses a dbus property. should it be asynchronous ?
     */
    void setAttributes( const QMap< QString, QString > &attributes );

    /**
     * FIXME: This methods accesses a dbus property. should it be asynchronous ?
     */
    bool isLocked() const;
    
    /**
     * FIXME: This methods accesses a dbus property. should it be asynchronous ?
     */
    QString label() const;
    
    /**
     * FIXME: This methods accesses a dbus property. should it be asynchronous ?
     */
    QDateTime createdTime() const;
    
    /**
     * FIXME: This methods accesses a dbus property. should it be asynchronous ?
     */
    QDateTime modifiedTime() const;
    
    /**
     * FIXME: This methods accesses a dbus property. should it be asynchronous ?
     */
    void setLabel( const QString &label );
    
private:
    friend class SecretItemPrivate;
    QSharedDataPointer< SecretItemPrivate > d;
};



};

#endif // KSECRETSITEM_H

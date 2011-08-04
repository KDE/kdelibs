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

#ifndef KSECRETSSECRET_P_H
#define KSECRETSSECRET_P_H

#include "ksecretsservicesecret.h"
#include "../daemon/frontend/secret/adaptors/secretstruct.h"

#include <QtDBus/QDBusObjectPath>
#include <QSharedData>

class SecretPrivate : public QSharedData {
public:
    SecretPrivate();
    SecretPrivate( const SecretPrivate& that );
    SecretPrivate( const SecretStruct& secretStruct );
    ~SecretPrivate();

    
    /**
     * This will obtain a properly encrpyted SecretStruct ready to be transmitted over the dbus 
     * to the daemon
     */
    bool toSecretStruct( SecretStruct &secretStruct ) const;
    static bool fromSecretStrut( const SecretStruct &secretStruct, SecretPrivate*& );
    
    bool operator == ( const SecretPrivate &that ) const;

    QString contentType;
    QVariant value;
};


#endif // KSECRETSSECRET_P_H

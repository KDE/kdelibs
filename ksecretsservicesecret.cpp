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

#include "ksecretsservicesecret.h"
#include "ksecretsservicesecret_p.h"
#include "dbusbackend.h"

namespace KSecretsService {

Secret::Secret() :
    d( new SecretPrivate() )
{
}

Secret::Secret( const Secret& that ) {
    // TODO: implement this
}

Secret::~Secret() {
    // TODO: implement this
}

QVariant Secret::value() const {
    // TODO: implement this
    return QVariant();
}

void Secret::setValue( const QVariant &value, const QString &contentType ) {
    // TODO: implement this
}

void Secret::setValue( const QVariant &value ) {
    // TODO: implement this
}

SecretPrivate::SecretPrivate() 
{
}

SecretPrivate::SecretPrivate( const SecretPrivate &that )
{
    contentType = that.contentType;
    value = that.value;
}
    
SecretPrivate::~SecretPrivate() 
{
}

bool SecretPrivate::toSecretStruct( SecretStruct &secretStruct ) const {
    secretStruct.m_session = DBusSession::sessionPath();
    secretStruct.m_contentType = contentType;
    return DBusSession::encrypt( value, secretStruct );
}

}; // namespace 

#include "ksecretsservicesecret.moc"

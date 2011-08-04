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

using namespace KSecretsService;

Secret::Secret() :
    d( new SecretPrivate() )
{
}

Secret::Secret(SecretPrivate* sp) :
    d( sp )
{
}

Secret::Secret( const Secret& that ) :
    d( that.d )
{
}

Secret::Secret( const QSharedDataPointer<SecretPrivate> &sp) :
    d( sp )
{
}

Secret::~Secret() {
}

QVariant Secret::value() const {
    return d->value;
}

void Secret::setValue( const QVariant &value, const QString &contentType ) {
    d->value = value;
    d->contentType = contentType;
}

void Secret::setValue( const QVariant &val ) {
    d->value = val;
    d->contentType = "QVariant";
}

bool Secret::operator==(const Secret& that) const
{
    return *d == *that.d;
}

SecretPrivate::SecretPrivate() 
{
}

SecretPrivate::SecretPrivate( const SecretPrivate &that )
{
    contentType = that.contentType;
    value = that.value;
}

SecretPrivate::SecretPrivate( const SecretStruct &that )
{
    value = that.m_value;
    contentType = that.m_contentType;
}

SecretPrivate::~SecretPrivate() 
{
}

bool SecretPrivate::toSecretStruct( SecretStruct &secretStruct ) const 
{
    secretStruct.m_session = DBusSession::sessionPath();
    secretStruct.m_contentType = contentType;
    return DBusSession::encrypt( value, secretStruct );
}

bool SecretPrivate::fromSecretStrut( const SecretStruct &secretStruct, SecretPrivate*& sp)
{
    bool result = false;
    sp = 0;
    QVariant value;
    if ( DBusSession::decrypt( secretStruct, value ) ) {
        sp = new SecretPrivate();
        sp->value = value;
        sp->contentType = secretStruct.m_contentType;
        result = true;
    }
    return result;
}

bool SecretPrivate::operator == ( const SecretPrivate &that )  const
{
    bool result = contentType == that.contentType;
    result &= value == that.value;
    return result;
}


#include "ksecretsservicesecret.moc"

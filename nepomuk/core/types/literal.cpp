/* This file is part of the Nepomuk-KDE libraries
    Copyright (c) 2007 Sebastian Trueg <trueg@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "literal.h"
#include "literal_p.h"

#include <QtCore/QHash>
#include <QtCore/QVariant>

#include <Soprano/Vocabulary/RDFS>

#include <kglobal.h>


namespace {
typedef QHash<QString, QVariant::Type> XmlSchemaMap;

K_GLOBAL_STATIC(XmlSchemaMap, s_xmlSchemaTypes)

void initXmlSchemaTypes() {
    if( s_xmlSchemaTypes->isEmpty() ) {
        s_xmlSchemaTypes->insert( "int", QVariant::Int );
        s_xmlSchemaTypes->insert( "integer", QVariant::Int );
        s_xmlSchemaTypes->insert( "negativeInteger", QVariant::Int );
        s_xmlSchemaTypes->insert( "decimal", QVariant::Int );
        s_xmlSchemaTypes->insert( "short", QVariant::Int );
        s_xmlSchemaTypes->insert( "long", QVariant::LongLong );
        s_xmlSchemaTypes->insert( "unsignedInt", QVariant::UInt );
        s_xmlSchemaTypes->insert( "unsignedShort", QVariant::UInt );
        s_xmlSchemaTypes->insert( "unsignedLong", QVariant::ULongLong );
        s_xmlSchemaTypes->insert( "boolean", QVariant::Bool );
        s_xmlSchemaTypes->insert( "double", QVariant::Double );
        s_xmlSchemaTypes->insert( "float", QVariant::Double );
        s_xmlSchemaTypes->insert( "string", QVariant::String );
        s_xmlSchemaTypes->insert( "date", QVariant::Date );
        s_xmlSchemaTypes->insert( "time", QVariant::Time );
        s_xmlSchemaTypes->insert( "dateTime", QVariant::DateTime );
        //    s_xmlSchemaTypes->insert( "", QVariant::Url );
    }
}

const XmlSchemaMap& xmlSchemaTypes()
{
    if(s_xmlSchemaTypes->isEmpty())
        initXmlSchemaTypes();
    return *s_xmlSchemaTypes;
}
}


Nepomuk::Types::Literal::Literal()
{
    d = new Private();
}


Nepomuk::Types::Literal::Literal( const Literal& other )
{
    d = other.d;
}


Nepomuk::Types::Literal::Literal( const QUrl& dataType )
{
    d = new Private();
    d->dataTypeUri = dataType;

    // now determine the QVariant type
    initXmlSchemaTypes();

    // check if it is a known type, otherwise leave it as QVariant::Invalid
    if ( dataType == Soprano::Vocabulary::RDFS::Literal() ) {
        d->dataType = QVariant::String;
    }
    else {
        QHash<QString, QVariant::Type>::const_iterator it = xmlSchemaTypes().constFind( dataType.fragment() );
        if ( it != xmlSchemaTypes().constEnd() ) {
            d->dataType = it.value();
        }
    }
}


Nepomuk::Types::Literal::~Literal()
{
}


Nepomuk::Types::Literal& Nepomuk::Types::Literal::operator=( const Literal& other )
{
    d = other.d;
    return *this;
}


QUrl Nepomuk::Types::Literal::dataTypeUri() const
{
    return d->dataTypeUri;
}


QVariant::Type Nepomuk::Types::Literal::dataType() const
{
    return d->dataType;
}


bool Nepomuk::Types::Literal::isValid() const
{
    return d->dataTypeUri.isValid();
}




Nepomuk::Literal::Literal()
{
    d = new Private();
}


Nepomuk::Literal::Literal( const Literal& other )
{
    d = other.d;
}


Nepomuk::Literal::Literal( const QUrl& dataType )
{
    d = new Private();
    d->dataTypeUri = dataType;

    // now determine the QVariant type
    initXmlSchemaTypes();

    // check if it is a known type, otherwise leave it as QVariant::Invalid
    QHash<QString, QVariant::Type>::const_iterator it = xmlSchemaTypes().constFind( dataType.fragment() );
    if ( it != xmlSchemaTypes().constEnd() ) {
        d->dataType = it.value();
    }
}


Nepomuk::Literal::~Literal()
{
}


Nepomuk::Literal& Nepomuk::Literal::operator=( const Literal& other )
{
    d = other.d;
    return *this;
}


const QUrl& Nepomuk::Literal::dataTypeUri() const
{
    return d->dataTypeUri;
}


QVariant::Type Nepomuk::Literal::dataType() const
{
    return d->dataType;
}

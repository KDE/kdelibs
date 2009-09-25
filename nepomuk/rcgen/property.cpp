/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING.LIB" for the exact licensing terms.
 */

#include "property.h"
#include "resourceclass.h"

#include <QtCore/QDebug>
#include <QtCore/QHash>
#include <QtCore/QRegExp>

#include <Soprano/Vocabulary/RDFS>

extern bool quiet;

Property::Property()
    : m_range( 0 ),
      m_isList( true ),
      m_domain( 0 ),
      m_inverseProperty( 0 )
{
}

// Property::Property( const QUrl& uri, const QString& type )
//     : m_uri( uri ),
//       m_type( type ),
//       m_isList( true ),
//       m_domain( 0 ),
//       m_inverseProperty( 0 )
// {
// }

void Property::setUri( const QUrl &uri )
{
    m_uri = uri;
}

QUrl Property::uri() const
{
    return m_uri;
}

void Property::setRange( ResourceClass* type )
{
    m_range = type;
}

void Property::setLiteralRange( const QString& range )
{
    m_literalRange = range;
}

ResourceClass* Property::range() const
{
    return m_range;
}

QString Property::literalRange() const
{
    return m_literalRange;
}

void Property::setComment( const QString &comment )
{
    m_comment = comment;
}

QString Property::comment() const
{
    return m_comment;
}

void Property::setIsList( bool isList )
{
    m_isList = isList;
}

bool Property::isList() const
{
    return m_isList;
}

void Property::setDomain( ResourceClass *domain )
{
    m_domain = domain;
}

ResourceClass* Property::domain( bool onlyReturnGeneratedClass ) const
{
    ResourceClass* domain = m_domain;
    if(onlyReturnGeneratedClass) {
        while( !domain->generateClass() &&
               domain->uri() != Soprano::Vocabulary::RDFS::Resource() )
            domain = domain->parentClass();
    }
    return domain;
}

void Property::setInverseProperty( Property *property )
{
    m_inverseProperty = property;
}

Property* Property::inverseProperty() const
{
    return m_inverseProperty;
}

QString Property::name() const
{
    //
    // many predicates are named "hasSomething"
    // we remove the "has" becasue setHasSomething sounds weird
    //
    const QString name = m_uri.toString().section( QRegExp( "[#/:]" ), -1 );
    if( name.toLower().startsWith( "has" ) )
        return name.mid( 3 );
    else
        return name;
}

QString Property::typeString( bool simple, const QString &nameSpace ) const
{
    QString t;
    if( !m_literalRange.isEmpty() ) {
        t = m_literalRange;
    }
    else {
        if( m_range->generateClass() )
            t = m_range->name();
        else
            t = m_range->parentClass( true )->name();
        if ( !nameSpace.isEmpty() )
            t.prepend( nameSpace + QLatin1String( "::" ) );
    }

    Q_ASSERT( !t.isEmpty() );

    if( !simple && m_isList ) {
        if( t == "QString" )
            return "QStringList";
        else
            return "QList<" + t + '>';
    }

    return t;
}

bool Property::hasSimpleType() const
{
    return ( m_range == 0 );
}

QString Property::literalTypeConversionMethod() const
{
    // for properties with cardinality == 1 we use a little hack since there will always be duplication of
    // data.
    if ( typeString(false) == "QStringList" ) {
        return QLatin1String("toStringList())");
    }
    else if ( typeString(true) == "QString" ) {
        return QLatin1String("toStringList() << QString() ).first()");
    }
    else if ( typeString(true) == "qint32" ) {
        return m_isList ? QLatin1String("toIntList())") : QLatin1String("toIntList() << 0 ).first()");
    }
    else if ( typeString(true) == "quint32" ) {
        return m_isList ? QLatin1String("toUnsignedIntList())") : QLatin1String("toUnsignedIntList() << 0 ).first()");
    }
    else if ( typeString(true) == "qint64" ) {
        return m_isList ? QLatin1String("toInt64List())") : QLatin1String("toInt64List() << 0 ).first()");
    }
    else if ( typeString(true) == "quint64" ) {
        return m_isList ? QLatin1String("toUnsignedInt64List())") : QLatin1String("toUnsignedInt64List() << 0 ).first()");
    }
    else if ( typeString(true) == "bool" ) {
        return m_isList ? QLatin1String("toBoolList())") : QLatin1String("toBoolList() << false ).first()");
    }
    else if ( typeString(true) == "double" ) {
        return m_isList ? QLatin1String("toDoubleList())") : QLatin1String("toDoubleList() << 0.0 ).first()");
    }
    else if ( typeString(true) == "QDateTime" ) {
        return m_isList ? QLatin1String("toDateTimeList())") : QLatin1String("toDateTimeList() << QDateTime() ).first()");
    }
    else if ( typeString(true) == "QDate" ) {
        return m_isList ? QLatin1String("toDateList())") : QLatin1String("toDateList() << QDate() ).first()");
    }
    else if ( typeString(true) == "QTime" ) {
        return m_isList ? QLatin1String("toTimeList())") : QLatin1String("toTimeList() << QTime() ).first()");
    }

    if ( !quiet )
        qDebug() << "Unknown type:" << typeString(true);

    return QString();
}

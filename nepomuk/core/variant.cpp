/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2009 Sebastian Trueg <trueg@kde.org>
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

#include "variant.h"
#include "resource.h"

#include <soprano/literalvalue.h>
#include <soprano/node.h>

#include <kdebug.h>

#include <QtCore/QVariant>


namespace {
    template<typename T1, typename T2> QList<T2> convertList( const QList<T1>& l ) {
        QList<T2> il;
        for( int i = 0; i < l.count(); ++i ) {
            il.append( static_cast<T2>( l[i] ) );
        }
        return il;
    }
}


class Nepomuk::Variant::Private
{
public:
    QVariant value;
};


Nepomuk::Variant::Variant()
    : d( new Private )
{
}


Nepomuk::Variant::~Variant()
{
    delete d;
}


Nepomuk::Variant::Variant( const Variant& other )
    : d( new Private )
{
    operator=( other );
}


Nepomuk::Variant::Variant( const QVariant& other )
    : d( new Private )
{
    if ( other.userType() == QVariant::Int ||
         other.userType() == QVariant::LongLong ||
         other.userType() == QVariant::UInt ||
         other.userType() == QVariant::ULongLong ||
         other.userType() == QVariant::Bool ||
         other.userType() == QVariant::Double ||
         other.userType() == QVariant::String ||
         other.userType() == QVariant::Date ||
         other.userType() == QVariant::Time ||
         other.userType() == QVariant::DateTime ||
         other.userType() == QVariant::Url ||
         other.userType() == qMetaTypeId<Resource>() ||
         other.userType() == qMetaTypeId<QList<int> >() ||
         other.userType() == qMetaTypeId<QList<qlonglong> >() ||
         other.userType() == qMetaTypeId<QList<uint> >() ||
         other.userType() == qMetaTypeId<QList<qulonglong> >() ||
         other.userType() == qMetaTypeId<QList<bool> >() ||
         other.userType() == qMetaTypeId<QList<double> >() ||
         other.userType() == QVariant::StringList ||
         other.userType() == qMetaTypeId<QList<QDate> >() ||
         other.userType() == qMetaTypeId<QList<QTime> >() ||
         other.userType() == qMetaTypeId<QList<QDateTime> >() ||
         other.userType() == qMetaTypeId<QList<QUrl> >() ||
         other.userType() == qMetaTypeId<QList<Resource> >() ) {
        d->value = other;
    }
}


Nepomuk::Variant::Variant( int i )
    : d( new Private )
{
    d->value.setValue( i );
}


Nepomuk::Variant::Variant( qlonglong i )
    : d( new Private )
{
    d->value.setValue( i );
}


Nepomuk::Variant::Variant( uint i )
    : d( new Private )
{
    d->value.setValue( i );
}


Nepomuk::Variant::Variant( qulonglong i )
    : d( new Private )
{
    d->value.setValue( i );
}


Nepomuk::Variant::Variant( bool b )
    : d( new Private )
{
    d->value.setValue( b );
}


Nepomuk::Variant::Variant( double v )
    : d( new Private )
{
    d->value.setValue( v );
}


Nepomuk::Variant::Variant( const char* string )
    : d( new Private )
{
    d->value.setValue( QString::fromLatin1(string) );
}


Nepomuk::Variant::Variant( const QString& string )
    : d( new Private )
{
    d->value.setValue( string );
}


Nepomuk::Variant::Variant( const QDate& date )
    : d( new Private )
{
    d->value.setValue( date );
}


Nepomuk::Variant::Variant( const QTime& time )
    : d( new Private )
{
    d->value.setValue( time );
}


Nepomuk::Variant::Variant( const QDateTime& datetime )
    : d( new Private )
{
    d->value.setValue( datetime );
}


Nepomuk::Variant::Variant( const QUrl& url )
    : d( new Private )
{
    d->value.setValue( url );
}


Nepomuk::Variant::Variant( const Nepomuk::Resource& r )
    : d( new Private )
{
    d->value.setValue( r );
}


Nepomuk::Variant::Variant( const QList<int>& i )
    : d( new Private )
{
    d->value.setValue( i );
}


Nepomuk::Variant::Variant( const QList<qlonglong>& i )
    : d( new Private )
{
    d->value.setValue( i );
}


Nepomuk::Variant::Variant( const QList<uint>& i )
    : d( new Private )
{
    d->value.setValue( i );
}


Nepomuk::Variant::Variant( const QList<qulonglong>& i )
    : d( new Private )
{
    d->value.setValue( i );
}


Nepomuk::Variant::Variant( const QList<bool>& b )
    : d( new Private )
{
    d->value.setValue( b );
}


Nepomuk::Variant::Variant( const QList<double>& v )
    : d( new Private )
{
    d->value.setValue( v );
}


Nepomuk::Variant::Variant( const QStringList& stringlist )
    : d( new Private )
{
    d->value.setValue( stringlist );
}


Nepomuk::Variant::Variant( const QList<QDate>& date )
    : d( new Private )
{
    d->value.setValue( date );
}


Nepomuk::Variant::Variant( const QList<QTime>& time )
    : d( new Private )
{
    d->value.setValue( time );
}


Nepomuk::Variant::Variant( const QList<QDateTime>& datetime )
    : d( new Private )
{
    d->value.setValue( datetime );
}


Nepomuk::Variant::Variant( const QList<QUrl>& url )
    : d( new Private )
{
    d->value.setValue( url );
}



Nepomuk::Variant::Variant( const QList<Resource>& r )
    : d( new Private )
{
    d->value.setValue( r );
}


Nepomuk::Variant::Variant( const QList<Variant>& vl )
    : d( new Private )
{
    foreach( const Variant& v, vl ) {
        append( v );
    }
}


Nepomuk::Variant& Nepomuk::Variant::operator=( const Variant& v )
{
    d->value = v.d->value;
    return *this;
}


Nepomuk::Variant& Nepomuk::Variant::operator=( int i )
{
    d->value.setValue( i );
    return *this;
}


Nepomuk::Variant& Nepomuk::Variant::operator=( qlonglong i )
{
    d->value.setValue( i );
    return *this;
}


Nepomuk::Variant& Nepomuk::Variant::operator=( uint i )
{
    d->value.setValue( i );
    return *this;
}


Nepomuk::Variant& Nepomuk::Variant::operator=( qulonglong i )
{
    d->value.setValue( i );
    return *this;
}


Nepomuk::Variant& Nepomuk::Variant::operator=( bool b )
{
    d->value.setValue( b );
    return *this;
}


Nepomuk::Variant& Nepomuk::Variant::operator=( double v )
{
    d->value.setValue( v );
    return *this;
}


Nepomuk::Variant& Nepomuk::Variant::operator=( const QString& string )
{
    d->value.setValue( string );
    return *this;
}


Nepomuk::Variant& Nepomuk::Variant::operator=( const QDate& date )
{
    d->value.setValue( date );
    return *this;
}


Nepomuk::Variant& Nepomuk::Variant::operator=( const QTime& time )
{
    d->value.setValue( time );
    return *this;
}


Nepomuk::Variant& Nepomuk::Variant::operator=( const QDateTime& datetime )
{
    d->value.setValue( datetime );
    return *this;
}


Nepomuk::Variant& Nepomuk::Variant::operator=( const QUrl& url )
{
    d->value.setValue( url );
    return *this;
}


Nepomuk::Variant& Nepomuk::Variant::operator=( const Resource& r )
{
    d->value.setValue( r );
    return *this;
}


Nepomuk::Variant& Nepomuk::Variant::operator=( const QList<int>& i )
{
    d->value.setValue( i );
    return *this;
}


Nepomuk::Variant& Nepomuk::Variant::operator=( const QList<qlonglong>& i )
{
    d->value.setValue( i );
    return *this;
}


Nepomuk::Variant& Nepomuk::Variant::operator=( const QList<uint>& i )
{
    d->value.setValue( i );
    return *this;
}


Nepomuk::Variant& Nepomuk::Variant::operator=( const QList<qulonglong>& i )
{
    d->value.setValue( i );
    return *this;
}


Nepomuk::Variant& Nepomuk::Variant::operator=( const QList<bool>& b )
{
    d->value.setValue( b );
    return *this;
}


Nepomuk::Variant& Nepomuk::Variant::operator=( const QList<double>& v )
{
    d->value.setValue( v );
    return *this;
}


Nepomuk::Variant& Nepomuk::Variant::operator=( const QStringList& stringlist )
{
    d->value.setValue( stringlist );
    return *this;
}


Nepomuk::Variant& Nepomuk::Variant::operator=( const QList<QDate>& date )
{
    d->value.setValue( date );
    return *this;
}


Nepomuk::Variant& Nepomuk::Variant::operator=( const QList<QTime>& time )
{
    d->value.setValue( time );
    return *this;
}


Nepomuk::Variant& Nepomuk::Variant::operator=( const QList<QDateTime>& datetime )
{
    d->value.setValue( datetime );
    return *this;
}


Nepomuk::Variant& Nepomuk::Variant::operator=( const QList<QUrl>& url )
{
    d->value.setValue( url );
    return *this;
}


Nepomuk::Variant& Nepomuk::Variant::operator=( const QList<Resource>& r )
{
    d->value.setValue( r );
    return *this;
}


void Nepomuk::Variant::append( int i )
{
    QList<int> l = toIntList();
    l.append( i );
    operator=( l );
}


void Nepomuk::Variant::append( qlonglong i )
{
    QList<qlonglong> l = toInt64List();
    l.append( i );
    operator=( l );
}


void Nepomuk::Variant::append( uint i )
{
    QList<uint> l = toUnsignedIntList();
    l.append( i );
    operator=( l );
}


void Nepomuk::Variant::append( qulonglong i )
{
    QList<qulonglong> l = toUnsignedInt64List();
    l.append( i );
    operator=( l );
}


void Nepomuk::Variant::append( bool b )
{
    QList<bool> l = toBoolList();
    l.append( b );
    operator=( l );
}


void Nepomuk::Variant::append( double d )
{
    QList<double> l = toDoubleList();
    l.append( d );
    operator=( l );
}


void Nepomuk::Variant::append( const QString& string )
{
    QStringList l = toStringList();
    l.append( string );
    operator=( l );
}


void Nepomuk::Variant::append( const QDate& date )
{
    QList<QDate> l = toDateList();
    l.append( date );
    operator=( l );
}


void Nepomuk::Variant::append( const QTime& time )
{
    QList<QTime> l = toTimeList();
    l.append( time );
    operator=( l );
}


void Nepomuk::Variant::append( const QDateTime& datetime )
{
    QList<QDateTime> l = toDateTimeList();
    l.append( datetime );
    operator=( l );
}


void Nepomuk::Variant::append( const QUrl& url )
{
    QList<QUrl> l = toUrlList();
    l.append( url );
    operator=( l );
}


void Nepomuk::Variant::append( const Resource& r )
{
    QList<Resource> l = toResourceList();
    if ( !l.contains( r ) ) {
        l.append( r );
        operator=( l );
    }
}


void Nepomuk::Variant::append( const Variant& v )
{
    if ( !isValid() ) {
        operator=( v );
    }
    else {
        if( v.simpleType() == QVariant::Int ) {
            operator=( toIntList() += v.toIntList() );
        }
        else if( v.simpleType() == QVariant::UInt ) {
            operator=( toUnsignedIntList() += v.toUnsignedIntList() );
        }
        else if( v.simpleType() == QVariant::LongLong ) {
            operator=( toInt64List() += v.toInt64List() );
        }
        else if( v.simpleType() == QVariant::ULongLong ) {
            operator=( toUnsignedInt64List() += v.toUnsignedInt64List() );
        }
        else if( v.simpleType() == QVariant::Bool ) {
            operator=( toBoolList() += v.toBoolList() );
        }
        else if( v.simpleType() == QVariant::Double ) {
            operator=( toDoubleList() += v.toDoubleList() );
        }
        else if( v.simpleType() == QVariant::String ) {
            operator=( toStringList() += v.toStringList() );
        }
        else if( v.simpleType() == QVariant::Date ) {
            operator=( toDateList() += v.toDateList() );
        }
        else if( v.simpleType() == QVariant::Time ) {
            operator=( toTimeList() += v.toTimeList() );
        }
        else if( v.simpleType() == QVariant::DateTime ) {
            operator=( toDateTimeList() += v.toDateTimeList() );
        }
        else if( v.simpleType() == QVariant::Url ) {
            operator=( toUrlList() += v.toUrlList() );
        }
        else if( v.simpleType() == qMetaTypeId<Resource>() ) {
            operator=( toResourceList() += v.toResourceList() );
        }
        else
            kDebug() << "(Variant::append) unknown type: " << v.simpleType();
    }
}


bool Nepomuk::Variant::isInt() const
{
    return( type() == QVariant::Int );
}


bool Nepomuk::Variant::isInt64() const
{
    return( type() == QVariant::LongLong );
}


bool Nepomuk::Variant::isUnsignedInt() const
{
    return( type() == QVariant::UInt );
}


bool Nepomuk::Variant::isUnsignedInt64() const
{
    return( type() == QVariant::ULongLong );
}


bool Nepomuk::Variant::isBool() const
{
    return( type() == QVariant::Bool );
}


bool Nepomuk::Variant::isDouble() const
{
    return( type() == QVariant::Double );
}


bool Nepomuk::Variant::isString() const
{
    return( type() == QVariant::String );
}


bool Nepomuk::Variant::isDate() const
{
    return( type() == QVariant::Date );
}


bool Nepomuk::Variant::isTime() const
{
    return( type() == QVariant::Time );
}


bool Nepomuk::Variant::isDateTime() const
{
    return( type() == QVariant::DateTime );
}


bool Nepomuk::Variant::isUrl() const
{
    return( type() == QVariant::Url );
}


bool Nepomuk::Variant::isResource() const
{
    return( type() == qMetaTypeId<Resource>() ||
            isUrl() );
}


bool Nepomuk::Variant::isIntList() const
{
    return( type() == qMetaTypeId<QList<int> >() );
}


bool Nepomuk::Variant::isUnsignedIntList() const
{
    return( type() == qMetaTypeId<QList<uint> >() );
}


bool Nepomuk::Variant::isInt64List() const
{
    return( type() == qMetaTypeId<QList<qlonglong> >() );
}


bool Nepomuk::Variant::isUnsignedInt64List() const
{
    return( type() == qMetaTypeId<QList<qulonglong> >() );
}


bool Nepomuk::Variant::isBoolList() const
{
    return( type() == qMetaTypeId<QList<bool> >() );
}


bool Nepomuk::Variant::isDoubleList() const
{
    return( type() == qMetaTypeId<QList<double> >() );
}


bool Nepomuk::Variant::isStringList() const
{
    return( type() == QVariant::StringList );
}


bool Nepomuk::Variant::isDateList() const
{
    return( type() == qMetaTypeId<QList<QDate> >() );
}


bool Nepomuk::Variant::isTimeList() const
{
    return( type() == qMetaTypeId<QList<QTime> >() );
}


bool Nepomuk::Variant::isDateTimeList() const
{
    return( type() == qMetaTypeId<QList<QDateTime> >() );
}


bool Nepomuk::Variant::isUrlList() const
{
    return( type() == qMetaTypeId<QList<QUrl> >() );
}


bool Nepomuk::Variant::isResourceList() const
{
    return( type() == qMetaTypeId<QList<Resource> >() ||
            isUrlList() );
}



int Nepomuk::Variant::toInt() const
{
    if(isList()) {
        QList<int> l = toIntList();
        if(!l.isEmpty())
            return l.first();
    }

    return d->value.toInt();
}


qlonglong Nepomuk::Variant::toInt64() const
{
    if(isList()) {
        QList<qlonglong> l = toInt64List();
        if(!l.isEmpty())
            return l.first();
    }

    return d->value.toLongLong();
}


uint Nepomuk::Variant::toUnsignedInt() const
{
    if(isList()) {
        QList<uint> l = toUnsignedIntList();
        if(!l.isEmpty())
            return l.first();
    }

    return d->value.toUInt();
}


qulonglong Nepomuk::Variant::toUnsignedInt64() const
{
    if(isList()) {
        QList<qulonglong> l = toUnsignedInt64List();
        if(!l.isEmpty())
            return l.first();
    }

    return d->value.toULongLong();
}


bool Nepomuk::Variant::toBool() const
{
    if(isList()) {
        QList<bool> l = toBoolList();
        if(!l.isEmpty())
            return l.first();
    }

    return d->value.toBool();
}


double Nepomuk::Variant::toDouble() const
{
    if(isList()) {
        QList<double> l = toDoubleList();
        if(!l.isEmpty())
            return l.first();
    }

    return d->value.toDouble();
}


QString Nepomuk::Variant::toString() const
{
//    kDebug() << "(Variant::toString() converting... " << QMetaType::typeName(type());
    if( isList() )
        return toStringList().join( "," );

    else if( isInt() )
        return QString::number( toInt() );
    else if( isInt64() )
        return QString::number( toInt64() );
    else if( isUnsignedInt() )
        return QString::number( toUnsignedInt() );
    else if( isUnsignedInt64() )
        return QString::number( toUnsignedInt64() );
    else if( isBool() )
        return ( toBool() ? QString("true") : QString("false" ) );
    else if( isDouble() )
        return QString::number( toDouble(), 'e', 10 );
    else if( isDate() )
        return Soprano::LiteralValue( toDate() ).toString();
    else if( isTime() )
        return Soprano::LiteralValue( toTime() ).toString();
    else if( isDateTime() )
        return Soprano::LiteralValue( toDateTime() ).toString();
    else if( isUrl() )
        return toUrl().toString();
    else if( isResource() ) {
        Resource r = toResource();
        if( !r.resourceUri().isEmpty() )
            return r.resourceUri().toString();
        else if( !r.identifiers().isEmpty() )
            return r.identifiers().first();
        else
            return QString();
    }
    else
        return d->value.value<QString>();
}


QDate Nepomuk::Variant::toDate() const
{
    if(isList()) {
        QList<QDate> l = toDateList();
        if(!l.isEmpty())
            return l.first();
    }
    return d->value.toDate();
}


QTime Nepomuk::Variant::toTime() const
{
    if(isList()) {
        QList<QTime> l = toTimeList();
        if(!l.isEmpty())
            return l.first();
    }
    return d->value.toTime();
}


QDateTime Nepomuk::Variant::toDateTime() const
{
    if(isList()) {
        QList<QDateTime> l = toDateTimeList();
        if(!l.isEmpty())
            return l.first();
    }
    return d->value.toDateTime();
}


QUrl Nepomuk::Variant::toUrl() const
{
    if(isList()) {
        QList<QUrl> l = toUrlList();
        if(!l.isEmpty())
            return l.first();
    }
    else if(type() == qMetaTypeId<Resource>()) {
        return toResource().resourceUri();
    }

    return d->value.toUrl();
}


Nepomuk::Resource Nepomuk::Variant::toResource() const
{
    if(isResourceList() || isUrlList()) {
        QList<Resource> l = toResourceList();
        if(!l.isEmpty())
            return l.first();
    }
    else if(type() == QVariant::Url) {
        return Resource(toUrl());
    }

    return d->value.value<Resource>();
}



QList<int> Nepomuk::Variant::toIntList() const
{
    if( isUnsignedInt() ||
        isInt() ||
        isUnsignedInt64() ||
        isInt64() ) {
        QList<int> l;
        l.append( toInt() );
        return l;
    }
    else if ( isUnsignedIntList() ) {
        return convertList<uint, int>( d->value.value<QList<uint> >() );
    }
    else if ( isUnsignedInt64List() ) {
        return convertList<qulonglong, int>( d->value.value<QList<qulonglong> >() );
    }
    else if ( isInt64List() ) {
        return convertList<qlonglong, int>( d->value.value<QList<qlonglong> >() );
    }
    else {
        return d->value.value<QList<int> >();
    }
}


QList<qlonglong> Nepomuk::Variant::toInt64List() const
{
    if( isUnsignedInt() ||
        isInt() ||
        isUnsignedInt64() ||
        isInt64() ) {
        QList<qlonglong> l;
        l.append( toInt64() );
        return l;
    }
    else if ( isIntList() ) {
        return convertList<int, qlonglong>( d->value.value<QList<int> >() );
    }
    else if ( isUnsignedIntList() ) {
        return convertList<uint, qlonglong>( d->value.value<QList<uint> >() );
    }
    else if ( isUnsignedInt64List() ) {
        return convertList<qulonglong, qlonglong>( d->value.value<QList<qulonglong> >() );
    }
    else {
        return d->value.value<QList<qlonglong> >();
    }
}


QList<uint> Nepomuk::Variant::toUnsignedIntList() const
{
    if( isUnsignedInt() ||
        isInt() ||
        isUnsignedInt64() ||
        isInt64() ) {
        QList<uint> l;
        l.append( toUnsignedInt() );
        return l;
    }
    else if ( isIntList() ) {
        return convertList<int, uint>( d->value.value<QList<int> >() );
    }
    else if ( isUnsignedInt64List() ) {
        return convertList<qulonglong, uint>( d->value.value<QList<qulonglong> >() );
    }
    else if ( isInt64List() ) {
        return convertList<qlonglong, uint>( d->value.value<QList<qlonglong> >() );
    }
    else {
        return d->value.value<QList<uint> >();
    }
}


QList<qulonglong> Nepomuk::Variant::toUnsignedInt64List() const
{
    if( isUnsignedInt() ||
        isInt() ||
        isUnsignedInt64() ||
        isInt64() ) {
        QList<qulonglong> l;
        l.append( toUnsignedInt() );
        return l;
    }
    else if ( isIntList() ) {
        return convertList<int, qulonglong>( d->value.value<QList<int> >() );
    }
    else if ( isUnsignedIntList() ) {
        return convertList<uint, qulonglong>( d->value.value<QList<uint> >() );
    }
    else if ( isInt64List() ) {
        return convertList<qlonglong, qulonglong>( d->value.value<QList<qlonglong> >() );
    }
    else {
        return d->value.value<QList<qulonglong> >();
    }
}


QList<bool> Nepomuk::Variant::toBoolList() const
{
    if( isBool() ) {
        QList<bool> l;
        l.append( toBool() );
        return l;
    }
    else
        return d->value.value<QList<bool> >();
}


QList<double> Nepomuk::Variant::toDoubleList() const
{
    if( isDouble() ) {
        QList<double> l;
        l.append( toDouble() );
        return l;
    }
    else
        return d->value.value<QList<double> >();
}


template<typename T> QStringList convertToStringList( const QList<T>& l )
{
    QStringList sl;
    QListIterator<T> it( l );
    while( it.hasNext() )
        sl.append( Nepomuk::Variant( it.next() ).toString() );
//   for( QList<T>::const_iterator it = l.constBegin(); it != l.constEnd(); ++it )
//     sl.append( Nepomuk::Variant( *it ).toString() );
    return sl;
}

QStringList Nepomuk::Variant::toStringList() const
{
    //  kDebug() << "(Variant::toStringList() converting... " << QMetaType::typeName(simpleType());
    if( !d->value.isValid() )
        return QStringList();

    if( !isList() )
        return QStringList( toString() );

    else if( isIntList() )
        return convertToStringList<int>( toIntList() );
    else if( isInt64List() )
        return convertToStringList<qlonglong>( toInt64List() );
    else if( isUnsignedIntList() )
        return convertToStringList<uint>( toUnsignedIntList() );
    else if( isUnsignedInt64List() )
        return convertToStringList<qulonglong>( toUnsignedInt64List() );
    else if( isBoolList() )
        return convertToStringList<bool>( toBoolList() );
    else if( isDoubleList() )
        return convertToStringList<double>( toDoubleList() );
    else if( isDateList() )
        return convertToStringList<QDate>( toDateList() );
    else if( isTimeList() )
        return convertToStringList<QTime>( toTimeList() );
    else if( isDateTimeList() )
        return convertToStringList<QDateTime>( toDateTimeList() );
    else if( isUrlList() )
        return convertToStringList<QUrl>( toUrlList() );
    else if( isResourceList() )
        return convertToStringList<Resource>( toResourceList() );
    else
        return d->value.value<QStringList>();
}


QList<QDate> Nepomuk::Variant::toDateList() const
{
    if( isDate() ) {
        QList<QDate> l;
        l.append( toDate() );
        return l;
    }
    else
        return d->value.value<QList<QDate> >();
}


QList<QTime> Nepomuk::Variant::toTimeList() const
{
    if( isTime() ) {
        QList<QTime> l;
        l.append( toTime() );
        return l;
    }
    else
        return d->value.value<QList<QTime> >();
}


QList<QDateTime> Nepomuk::Variant::toDateTimeList() const
{
    if( isDateTime() ) {
        QList<QDateTime> l;
        l.append( toDateTime() );
        return l;
    }
    else
        return d->value.value<QList<QDateTime> >();
}


QList<QUrl> Nepomuk::Variant::toUrlList() const
{
    if( type() == qMetaTypeId<Resource>() ||
        type() == QVariant::Url ) {
        QList<QUrl> l;
        l.append( toUrl() );
        return l;
    }
    else if( type() == qMetaTypeId<QList<Resource> >() ) {
        QList<QUrl> l;
        QList<Resource> rl = toResourceList();
        foreach(const Resource& r, rl)
            l << r.resourceUri();
        return l;
    }
    else {
        return d->value.value<QList<QUrl> >();
    }
}


QList<Nepomuk::Resource> Nepomuk::Variant::toResourceList() const
{
    if( type() == qMetaTypeId<Resource>() ||
        type() == QVariant::Url ) {
        QList<Resource> l;
        l.append( toResource() );
        return l;
    }
    else if( type() == qMetaTypeId<QList<QUrl> >() ) {
        QList<QUrl> urls = toUrlList();
        QList<Resource> l;
        foreach(const QUrl& url, urls)
            l << Resource(url);
        return l;
    }
    else {
        return d->value.value<QList<Resource> >();
    }
}


QList<Nepomuk::Variant> Nepomuk::Variant::toVariantList() const
{
    QList<Variant> l;

    switch( simpleType() ) {
    case QVariant::Int:
        foreach( int i, toIntList() ) {
            l.append( Variant(i) );
        }
        break;

    case QVariant::LongLong:
        foreach( qlonglong i, toInt64List() ) {
            l.append( Variant(i) );
        }
        break;

    case QVariant::UInt:
        foreach( uint i, toUnsignedIntList() ) {
            l.append( Variant(i) );
        }
        break;

    case QVariant::ULongLong:
        foreach( qulonglong i, toUnsignedInt64List() ) {
            l.append( Variant(i) );
        }
        break;

    case QVariant::Bool:
        foreach( bool i, toBoolList() ) {
            l.append( Variant(i) );
        }
        break;

    case QVariant::Double:
        foreach( double i, toDoubleList() ) {
            l.append( Variant(i) );
        }
        break;

    case QVariant::Date:
        foreach( const QDate& i, toDateList() ) {
            l.append( Variant(i) );
        }
        break;

    case QVariant::Time:
        foreach( const QTime& i, toTimeList() ) {
            l.append( Variant(i) );
        }
        break;

    case QVariant::DateTime:
        foreach( const QDateTime& i, toDateTimeList() ) {
            l.append( Variant(i) );
        }
        break;

    case QVariant::Url:
        foreach( const QUrl& i, toUrlList() ) {
            l.append( Variant(i) );
        }
        break;

    default:
        if( simpleType() == qMetaTypeId<Resource>()) {
            foreach( const Resource& i, toResourceList() ) {
                l.append( Variant(i) );
            }
        }
        else {
            foreach( const QString& i, toStringList() ) {
                l.append( Variant(i) );
            }
            break;
        }
    }

    return l;
}


bool Nepomuk::Variant::isList() const
{
    return( isIntList() ||
            isInt64List() ||
            isUnsignedIntList() ||
            isUnsignedInt64List() ||
            isBoolList() ||
            isDoubleList() ||
            isStringList() ||
            isDateList() ||
            isTimeList() ||
            isDateTimeList() ||
            isUrlList() ||
            isResourceList() );
}


int Nepomuk::Variant::type() const
{
    return d->value.userType();
}


int Nepomuk::Variant::simpleType() const
{
    if( isIntList() )
        return QVariant::Int;
    else if( isInt64List() )
        return QVariant::LongLong;
    else if( isUnsignedIntList() )
        return QVariant::UInt;
    else if( isUnsignedInt64List() )
        return QVariant::ULongLong;
    else if( isBoolList() )
        return QVariant::Bool;
    else if( isDoubleList() )
        return QVariant::Double;
    else if( isStringList() )
        return QVariant::String;
    else if( isDateList() )
        return QVariant::Date;
    else if( isTimeList() )
        return QVariant::Time;
    else if( isDateTimeList() )
        return QVariant::DateTime;
    else if( isUrlList() )
        return QVariant::Url;
    else if( isResourceList() )
        return qMetaTypeId<Resource>();
    else
        return d->value.userType();
}


// static
Nepomuk::Variant Nepomuk::Variant::fromString( const QString& value, int type )
{
    // first check the types that are not supported by Soprano since they are not literal types
    if( type == qMetaTypeId<Resource>() ) {
        return Variant( Resource( value ) );
    }
    else if ( type == int( QVariant::Url ) ) {
        return Variant( QUrl( value ) );
    }

    // let Soprano do the rest
    else {
        return Variant( Soprano::LiteralValue::fromString( value, ( QVariant::Type )type ).variant() );
    }
}


// static
Nepomuk::Variant Nepomuk::Variant::fromNode( const Soprano::Node& node )
{
    //
    // We cannot put in Resource objects here since then nie:url file:/ URLs would
    // get converted back to the actual resource URIs which would be useless.
    // That is why Variant treats QUrl and Resource pretty much as similar.
    //
    if ( node.isResource() ) {
        return Nepomuk::Variant( node.uri() );
    }
    else if ( node.isLiteral() ) {
        return Nepomuk::Variant( node.literal().variant() );
    }
    else {
        return Nepomuk::Variant();
    }
}


bool Nepomuk::Variant::operator==( const Variant& other ) const
{
    // we handle the special case of Urls and Resources before
    // comparing the simple type
    if( isUrl() || isUrlList() )
        return other.toUrlList() == toUrlList();
    else if( isResource() || isResourceList() )
        return other.toResourceList() == toResourceList();

    if( other.simpleType() != this->simpleType() )
        return false;

    if( isInt() || isIntList() )
        return other.toIntList() == toIntList();
    else if( isInt64() || isInt64List() )
        return other.toInt64List() == toInt64List();
    else if( isUnsignedInt() || isUnsignedIntList() )
        return other.toUnsignedIntList() == toUnsignedIntList();
    else if( isUnsignedInt64() || isUnsignedInt64List() )
        return other.toUnsignedInt64List() == toUnsignedInt64List();
    else if( isBool() || isBoolList() )
        return other.toBoolList() == toBoolList();
    else if( isDouble() || isDoubleList() )
        return other.toDoubleList() == toDoubleList();
    else if( isString() || isStringList() )
        return other.d->value.value<QStringList>() == d->value.value<QStringList>();
    else if( isDate() || isDateList() )
        return other.toDateList() == toDateList();
    else if( isTime() || isTimeList() )
        return other.toTimeList() == toTimeList();
    else if( isDateTime() || isDateTimeList() )
        return other.toDateTimeList() == toDateTimeList();
    else
        return ( d->value == other.d->value );
}


bool Nepomuk::Variant::operator!=( const Variant& other ) const
{
    return !operator==( other );
}


QVariant Nepomuk::Variant::variant() const
{
    return d->value;
}


bool Nepomuk::Variant::isValid() const
{
    return d->value.isValid();
}


QDebug operator<<( QDebug dbg, const Nepomuk::Variant& v )
{
    if( v.isList() )
        dbg.nospace() << "Nepomuk::Variant(" << v.toStringList() << "@list)";
    else if( v.isResource() )
        dbg.nospace() << "Nepomuk::Variant(Nepomuk::Resource(" << v.toString() << "))";
    else
        dbg.nospace() << "Nepomuk::Variant(" << v.variant() << ")";
    return dbg;
}

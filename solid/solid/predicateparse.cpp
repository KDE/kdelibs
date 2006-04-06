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

extern "C"
{
#include "predicateparse.h"

void PredicateParse_mainParse( const char *_code );
}

#include "predicate.h"

#include <cstdlib>

#include <QStringList>

#include <kdebug.h>

static KDEHW::Predicate *s_result = 0;

KDEHW::Predicate KDEHW::Predicate::fromString( const QString &predicate )
{
    PredicateParse_mainParse( predicate.toAscii() );

    if ( s_result == 0 )
    {
        return Predicate();
    }
    else
    {
        Predicate result( *s_result );
        delete s_result;
        s_result = 0;
        return result;
    }
}


void PredicateParse_setResult( void *result )
{
    s_result = (KDEHW::Predicate*) result;
}

void PredicateParse_errorDetected()
{
    if ( s_result != 0 )
    {
        delete s_result;
        s_result = 0;
    }
}

void *PredicateParse_newAtom( char *capability, char *property, void *value )
{
    QString cap( capability );
    QString prop( property );
    QVariant *val = (QVariant*)value;

    KDEHW::Predicate *result = new KDEHW::Predicate( cap, prop, *val );

    delete val;
    free( capability );
    free( property );

    return result;
}


void *PredicateParse_newAnd( void *pred1, void *pred2 )
{
    KDEHW::Predicate *result = new KDEHW::Predicate();

    KDEHW::Predicate *p1 = (KDEHW::Predicate*)pred1;
    KDEHW::Predicate *p2 = (KDEHW::Predicate*)pred2;

    *result = *p1 & *p2;

    delete p1;
    delete p2;

    return result;
}


void *PredicateParse_newOr( void *pred1, void *pred2 )
{
    KDEHW::Predicate *result = new KDEHW::Predicate();

    KDEHW::Predicate *p1 = (KDEHW::Predicate*)pred1;
    KDEHW::Predicate *p2 = (KDEHW::Predicate*)pred2;

    *result = *p1 | *p2;

    delete p1;
    delete p2;

    return result;
}


void *PredicateParse_newStringValue( char *val )
{
    QString s( val );

    free( val );

    return new QVariant( s );
}


void *PredicateParse_newBoolValue( int val )
{
    bool b = ( val != 0 );
    return new QVariant( b );
}


void *PredicateParse_newNumValue( int val )
{
    return new QVariant( val );
}


void *PredicateParse_newDoubleValue( double val )
{
    return new QVariant( val );
}


void *PredicateParse_newEmptyStringListValue()
{
    return new QVariant( QStringList() );
}


void *PredicateParse_newStringListValue( char *name )
{
    QStringList list;
    list << QString( name );

    free( name );

    return new QVariant( list );
}


void *PredicateParse_appendStringListValue( char *name, void *list )
{
    QVariant *variant = (QVariant*)list;

    QStringList new_list = variant->toStringList();

    new_list << QString( name );

    delete variant;
    free( name );

    return new QVariant( new_list );
}

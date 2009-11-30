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

#include "resourceclass.h"

#include <QtCore/QRegExp>

#include <Soprano/Vocabulary/RDFS>

ResourceClass::ResourceClass()
    : m_parentClass( 0 ),
      m_generateClass( false )
{
}

ResourceClass::ResourceClass( const QUrl& uri )
    : m_uri( uri ),
      m_parentClass( 0 ),
      m_generateClass( false )
{
}

ResourceClass::~ResourceClass()
{
}

void ResourceClass::setUri( const QUrl &uri )
{
    m_uri = uri;
}

QUrl ResourceClass::uri() const
{
    return m_uri;
}

void ResourceClass::setComment( const QString &comment )
{
    m_comment = comment;
}

QString ResourceClass::comment() const
{
    return m_comment;
}

void ResourceClass::setParentResource( ResourceClass* parent )
{
    m_parentClass = parent;
}

ResourceClass* ResourceClass::parentClass( bool considerGenerateClass ) const
{
    ResourceClass* parent = m_parentClass;
    if( considerGenerateClass && !parent->generateClass() ) {
        // first check for another "top-level" parent class to be generated
        foreach( ResourceClass* rc, m_allParentResources ) {
            if( rc->generateClass() &&
                rc->uri() != Soprano::Vocabulary::RDFS::Resource() ) {
                return rc;
            }
        }
        // nothing found -> just use the first
        while( !parent->generateClass() &&
               parent->uri() != Soprano::Vocabulary::RDFS::Resource() )
            parent = parent->parentClass();
    }
    return parent;
}

void ResourceClass::addParentResource( ResourceClass* parent )
{
    m_allParentResources.append( parent );
}

QList<ResourceClass*> ResourceClass::allParentResources() const
{
    return m_allParentResources;
}

void ResourceClass::addProperty( Property* property )
{
    m_properties.append( property );
}

Property::ConstPtrList ResourceClass::allProperties() const
{
    return m_properties;
}

void ResourceClass::addReverseProperty( Property* property )
{
    m_reverseProperties.append( property );
}

Property::ConstPtrList ResourceClass::allReverseProperties() const
{
    return m_reverseProperties;
}

void ResourceClass::setGenerateClass( bool generate )
{
    m_generateClass = generate;
}

bool ResourceClass::generateClass() const
{
    return m_generateClass;
}

QString ResourceClass::name( const QString &nameSpace ) const
{
    QString s = m_uri.toString().section( QRegExp( "[#/:]" ), -1 );
    if( !nameSpace.isEmpty() )
        s.prepend( nameSpace + "::" );
    return s;
}

QString ResourceClass::headerName() const
{
    return name().toLower() + ".h";
}

QString ResourceClass::sourceName() const
{
    return name().toLower() + ".cpp";
}

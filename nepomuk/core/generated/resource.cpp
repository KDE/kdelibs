/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "resource.h"
#include "resourcedata.h"
#include "resourcemanager.h"
#include "tools.h"

#include <klocale.h>
#include <kdebug.h>

// FIXME: let the code generator do this
#include "tag.h"


Nepomuk::Resource::Resource()
{
    // invalid data
    m_data = new ResourceData();
    m_data->ref();
}


Nepomuk::Resource::Resource( const Nepomuk::Resource& res )
{
    m_data = res.m_data;
    m_data->ref();
}


Nepomuk::Resource::Resource( const QString& uri, const QString& type )
{
    m_data = ResourceData::data( uri, type );
    m_data->ref();
}


Nepomuk::Resource::Resource( const QUrl& uri, const QUrl& type )
{
    m_data = ResourceData::data( uri, type );
    m_data->ref();
}


Nepomuk::Resource::Resource( Nepomuk::ResourceData* data )
{
    m_data = data;
    data->ref();
}


Nepomuk::Resource::~Resource()
{
    if( m_data->deref() == 0 ) {
        m_data->deleteData();
    }
}


Nepomuk::Resource& Nepomuk::Resource::operator=( const Resource& res )
{
    if( m_data != res.m_data ) {
        if ( m_data->deref() == 0 ) {
            m_data->deleteData();
        }
        m_data = res.m_data;
        m_data->ref();
    }

    return *this;
}


QString Nepomuk::Resource::uri() const
{
    m_data->determineUri();
    return m_data->uri();
}


QString Nepomuk::Resource::type() const
{
    m_data->determineUri();
    return m_data->type().toString();
}


QString Nepomuk::Resource::className() const
{
    return type().section( QRegExp( "[#:]" ), -1 );
}


QHash<QString, Nepomuk::Variant> Nepomuk::Resource::allProperties() const
{
    return m_data->allProperties();
}


bool Nepomuk::Resource::hasProperty( const QString& uri ) const
{
    return m_data->hasProperty( uri );
}


Nepomuk::Variant Nepomuk::Resource::property( const QString& uri ) const
{
    return m_data->property( uri );
}


void Nepomuk::Resource::setProperty( const QString& uri, const Nepomuk::Variant& value )
{
    m_data->setProperty( uri, value );
}


void Nepomuk::Resource::removeProperty( const QString& uri )
{
    m_data->removeProperty( uri );
}


void Nepomuk::Resource::remove()
{
    m_data->remove();
}


bool Nepomuk::Resource::exists() const
{
    return m_data->exists();
}


bool Nepomuk::Resource::isValid() const
{
    return m_data->isValid();
}


bool Nepomuk::Resource::operator==( const Resource& other ) const
{
    if( this == &other )
        return true;

    if( this->m_data == other.m_data )
        return true;

    m_data->determineUri();
    other.m_data->determineUri();
    return uri() == other.uri();
}


QString Nepomuk::errorString( ErrorCode code )
{
    switch( code ) {
    case NoError:
        return i18n("Success");
    case CommunicationError:
        return i18n("Communication error");
    case InvalidType:
        return i18n("Invalid type in Database");
    default:
        return i18n("Unknown error");
    }
}

// do not remove, will be replaced with method definitions by the Nepomuk class generator
QString Nepomuk::Resource::description() const
{
    return property( "http://semanticdesktop.org/ontologies/2007/03/31/nao#description" ).toString();
}

void Nepomuk::Resource::setDescription( const QString& value )
{
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#description", Variant( value ) );
}

QString Nepomuk::Resource::descriptionUri()
{
    return "http://semanticdesktop.org/ontologies/2007/03/31/nao#description";
}

QStringList Nepomuk::Resource::identifiers() const
{
    return property( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasIdentifier" ).toStringList();
}

void Nepomuk::Resource::setIdentifiers( const QStringList& value )
{
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasIdentifier", Variant( value ) );
}

void Nepomuk::Resource::addIdentifier( const QString& value )
{
    Variant v = property( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasIdentifier" );
    v.append( value );
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasIdentifier", v );
}

QString Nepomuk::Resource::identifierUri()
{
    return "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasIdentifier";
}

QStringList Nepomuk::Resource::altLabels() const
{
    return property( "http://semanticdesktop.org/ontologies/2007/03/31/nao#altLabel" ).toStringList();
}

void Nepomuk::Resource::setAltLabels( const QStringList& value )
{
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#altLabel", Variant( value ) );
}

void Nepomuk::Resource::addAltLabel( const QString& value )
{
    Variant v = property( "http://semanticdesktop.org/ontologies/2007/03/31/nao#altLabel" );
    v.append( value );
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#altLabel", v );
}

QString Nepomuk::Resource::altLabelUri()
{
    return "http://semanticdesktop.org/ontologies/2007/03/31/nao#altLabel";
}

QList<Nepomuk::Resource> Nepomuk::Resource::annotations() const
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    return convertResourceList<Resource>( property( "http://semanticdesktop.org/ontologies/2007/03/31/nao#annotation" ).toResourceList() );
}

void Nepomuk::Resource::setAnnotations( const QList<Nepomuk::Resource>& value )
{
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#annotation", Variant( value ) );
}

void Nepomuk::Resource::addAnnotation( const Nepomuk::Resource& value )
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    Variant v = property( "http://semanticdesktop.org/ontologies/2007/03/31/nao#annotation" );
    v.append( Resource( value ) );
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#annotation", v );
}

QString Nepomuk::Resource::annotationUri()
{
    return "http://semanticdesktop.org/ontologies/2007/03/31/nao#annotation";
}

QList<Nepomuk::Tag> Nepomuk::Resource::tags() const
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    return convertResourceList<Tag>( property( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasTag" ).toResourceList() );
}

void Nepomuk::Resource::setTags( const QList<Nepomuk::Tag>& value )
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    QList<Resource> l;
    for( QList<Tag>::const_iterator it = value.constBegin();
         it != value.constEnd(); ++it ) {
        l.append( Resource( (*it) ) );
    }
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasTag", Variant( l ) );
}

void Nepomuk::Resource::addTag( const Nepomuk::Tag& value )
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    Variant v = property( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasTag" );
    v.append( Resource( value ) );
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasTag", v );
}

QString Nepomuk::Resource::tagUri()
{
    return "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasTag";
}

QList<Nepomuk::Resource> Nepomuk::Resource::topics() const
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    return convertResourceList<Resource>( property( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasTopic" ).toResourceList() );
}

void Nepomuk::Resource::setTopics( const QList<Nepomuk::Resource>& value )
{
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasTopic", Variant( value ) );
}

void Nepomuk::Resource::addTopic( const Nepomuk::Resource& value )
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    Variant v = property( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasTopic" );
    v.append( Resource( value ) );
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasTopic", v );
}

QString Nepomuk::Resource::topicUri()
{
    return "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasTopic";
}

QList<Nepomuk::Resource> Nepomuk::Resource::isTopicOfs() const
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    return convertResourceList<Resource>( property( "http://semanticdesktop.org/ontologies/2007/03/31/nao#isTopicOf" ).toResourceList() );
}

void Nepomuk::Resource::setIsTopicOfs( const QList<Nepomuk::Resource>& value )
{
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#isTopicOf", Variant( value ) );
}

void Nepomuk::Resource::addIsTopicOf( const Nepomuk::Resource& value )
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    Variant v = property( "http://semanticdesktop.org/ontologies/2007/03/31/nao#isTopicOf" );
    v.append( Resource( value ) );
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#isTopicOf", v );
}

QString Nepomuk::Resource::isTopicOfUri()
{
    return "http://semanticdesktop.org/ontologies/2007/03/31/nao#isTopicOf";
}

QList<Nepomuk::Resource> Nepomuk::Resource::isRelateds() const
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    return convertResourceList<Resource>( property( "http://semanticdesktop.org/ontologies/2007/03/31/nao#isRelated" ).toResourceList() );
}

void Nepomuk::Resource::setIsRelateds( const QList<Nepomuk::Resource>& value )
{
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#isRelated", Variant( value ) );
}

void Nepomuk::Resource::addIsRelated( const Nepomuk::Resource& value )
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    Variant v = property( "http://semanticdesktop.org/ontologies/2007/03/31/nao#isRelated" );
    v.append( Resource( value ) );
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#isRelated", v );
}

QString Nepomuk::Resource::isRelatedUri()
{
    return "http://semanticdesktop.org/ontologies/2007/03/31/nao#isRelated";
}

QString Nepomuk::Resource::label() const
{
    return property( "http://semanticdesktop.org/ontologies/2007/03/31/nao#label" ).toString();
}

void Nepomuk::Resource::setLabel( const QString& value )
{
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#label", Variant( value ) );
}

QString Nepomuk::Resource::labelUri()
{
    return "http://semanticdesktop.org/ontologies/2007/03/31/nao#label";
}

quint32 Nepomuk::Resource::rating() const
{
    return property( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasRating" ).toUnsignedInt();
}

void Nepomuk::Resource::setRating( const quint32& value )
{
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasRating", Variant( value ) );
}

QString Nepomuk::Resource::ratingUri()
{
    return "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasRating";
}

QStringList Nepomuk::Resource::symbols() const
{
    return property( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasSymbol" ).toStringList();
}

void Nepomuk::Resource::setSymbols( const QStringList& value )
{
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasSymbol", Variant( value ) );
}

void Nepomuk::Resource::addSymbol( const QString& value )
{
    Variant v = property( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasSymbol" );
    v.append( value );
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasSymbol", v );
}

QString Nepomuk::Resource::symbolUri()
{
    return "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasSymbol";
}

QList<Nepomuk::Resource> Nepomuk::Resource::annotationOf() const
{
    return convertResourceList<Resource>( ResourceManager::instance()->allResourcesWithProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#annotation", *this ) );
}

QList<Nepomuk::Resource> Nepomuk::Resource::isRelatedOf() const
{
    return convertResourceList<Resource>( ResourceManager::instance()->allResourcesWithProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#isRelated", *this ) );
}

QList<Nepomuk::Resource> Nepomuk::Resource::allResources()
{
    return Nepomuk::convertResourceList<Resource>( ResourceManager::instance()->allResourcesOfType( "http://www.w3.org/2000/01/rdf-schema#Resource" ) );
}



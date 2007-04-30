/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006 Sebastian Trueg <trueg@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#include "resource.h"
#include "resourcedata.h"
#include "resourcemanager.h"
#include "tools.h"

#include <klocale.h>
#include <kdebug.h>

// FIXME: let the code generator do this
#include <kmetadata/tag.h>


Nepomuk::KMetaData::Resource::Resource()
{
    // invalid data
    m_data = new ResourceData();
    m_data->ref();
}


Nepomuk::KMetaData::Resource::Resource( const Nepomuk::KMetaData::Resource& res )
{
    m_data = res.m_data;
    m_data->ref();
}


Nepomuk::KMetaData::Resource::Resource( const QString& uri, const QString& type )
{
    m_data = ResourceData::data( uri, type );
    m_data->ref();
}


Nepomuk::KMetaData::Resource::Resource( Nepomuk::KMetaData::ResourceData* data )
{
    m_data = data;
    data->ref();
}


Nepomuk::KMetaData::Resource::~Resource()
{
    if( m_data->deref() == 0 &&
        !ResourceManager::instance()->autoSync() ) {
        m_data->deleteData();
    }
}


Nepomuk::KMetaData::Resource& Nepomuk::KMetaData::Resource::operator=( const Resource& res )
{
    if( m_data != res.m_data ) {
        m_data->deref();
        m_data = res.m_data;
        m_data->ref();
    }

    return *this;
}


const QString& Nepomuk::KMetaData::Resource::uri() const
{
    return m_data->uri();
}


const QString& Nepomuk::KMetaData::Resource::type() const
{
    m_data->init();
    return m_data->type();
}


// QStringList Nepomuk::KMetaData::Resource::getIdentifiers() const
// {
//   // TODO: do not hardcode the hasIdentifier URI here!
//   QStringList il = m_data->getProperty( "http://nepomuk-kde.semanticdesktop.org/ontology/nkde-0.1#hasIdentifier" ).toStringList();
//   if( !m_data->kickoffUriOrId().isEmpty() && !il.contains( m_data->kickoffUriOrId() ) )
//     il.append( m_data->kickoffUriOrId() );
//   return il;
// }


QString Nepomuk::KMetaData::Resource::className() const
{
    return type().section( QRegExp( "[#:]" ), -1 );
}


int Nepomuk::KMetaData::Resource::sync()
{
    m_data->init();

    m_data->startSync();

    bool success = ( m_data->determineUri() &&
                     m_data->determinePropertyUris() &&
                     m_data->merge() &&
                     m_data->save() );

    m_data->endSync( success );

    return ( success ? 0 : -1 );
}


QHash<QString, Nepomuk::KMetaData::Variant> Nepomuk::KMetaData::Resource::allProperties() const
{
    m_data->init();
    return m_data->allProperties();
}


bool Nepomuk::KMetaData::Resource::hasProperty( const QString& uri ) const
{
    m_data->init();
    return m_data->hasProperty( uri );
}


Nepomuk::KMetaData::Variant Nepomuk::KMetaData::Resource::getProperty( const QString& uri ) const
{
    m_data->init();
    return m_data->getProperty( uri );
}


void Nepomuk::KMetaData::Resource::setProperty( const QString& uri, const Nepomuk::KMetaData::Variant& value )
{
    //  m_data->init();
    m_data->setProperty( uri, value );
}


void Nepomuk::KMetaData::Resource::removeProperty( const QString& uri )
{
    m_data->init();
    m_data->removeProperty( uri );
}


void Nepomuk::KMetaData::Resource::remove()
{
    //  m_data->init();
    m_data->remove();
}


void Nepomuk::KMetaData::Resource::revive()
{
    m_data->revive();
}


bool Nepomuk::KMetaData::Resource::isProperty( const QString& uri ) const
{
    return !ResourceManager::instance()->allResourcesWithProperty( uri, *this ).isEmpty();
}


bool Nepomuk::KMetaData::Resource::modified() const
{
    return m_data->modified();
}


bool Nepomuk::KMetaData::Resource::inSync() const
{
    return m_data->inSync();
}


bool Nepomuk::KMetaData::Resource::exists() const
{
    return m_data->exists();
}


bool Nepomuk::KMetaData::Resource::isValid() const
{
    return m_data->isValid();
}


bool Nepomuk::KMetaData::Resource::operator==( const Resource& other ) const
{
    if( this == &other )
        return true;

    if( this->m_data == other.m_data )
        return true;

    m_data->determineUri();
    other.m_data->determineUri();
    return uri() == other.uri();

    //    return( *m_data == *other.m_data );
}


QString Nepomuk::KMetaData::errorString( int code )
{
    switch( code ) {
    case ERROR_SUCCESS:
        return i18n("Success");
    case ERROR_COMMUNICATION:
        return i18n("Communication error");
    case ERROR_INVALID_TYPE:
        return i18n("Invalid type in Database");
    default:
        return i18n("Unknown error");
    }
}

// do not remove, will be replaced with method definitions by the KMetaData class generator
QStringList Nepomuk::KMetaData::Resource::getSymbols() const
{
    return getProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#symbol" ).toStringList();
}

void Nepomuk::KMetaData::Resource::setSymbols( const QStringList& value )
{
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#symbol", Variant( value ) );
}

void Nepomuk::KMetaData::Resource::addSymbol( const QString& value )
{
    Variant v = getProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#symbol" );
    v.append( value );
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#symbol", v );
}

QString Nepomuk::KMetaData::Resource::symbolUri()
{
    return "http://semanticdesktop.org/ontologies/2007/03/31/nao#symbol";
}

quint32 Nepomuk::KMetaData::Resource::getRating() const
{
    return getProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#rating" ).value<quint32>();
}

void Nepomuk::KMetaData::Resource::setRating( const quint32& value )
{
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#rating", Variant( value ) );
}

QString Nepomuk::KMetaData::Resource::ratingUri()
{
    return "http://semanticdesktop.org/ontologies/2007/03/31/nao#rating";
}

QStringList Nepomuk::KMetaData::Resource::getIdentifiers() const
{
    return getProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#altIdentifier" ).toStringList();
}

void Nepomuk::KMetaData::Resource::setIdentifiers( const QStringList& value )
{
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#altIdentifier", Variant( value ) );
}

void Nepomuk::KMetaData::Resource::addIdentifier( const QString& value )
{
    Variant v = getProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#altIdentifier" );
    v.append( value );
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#altIdentifier", v );
}

QString Nepomuk::KMetaData::Resource::IdentifierUri()
{
    return "http://semanticdesktop.org/ontologies/2007/03/31/nao#altIdentifier";
}

QStringList Nepomuk::KMetaData::Resource::getPreflabels() const
{
    return getProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#preflabel" ).toStringList();
}

void Nepomuk::KMetaData::Resource::setPreflabels( const QStringList& value )
{
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#preflabel", Variant( value ) );
}

void Nepomuk::KMetaData::Resource::addPreflabel( const QString& value )
{
    Variant v = getProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#preflabel" );
    v.append( value );
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#preflabel", v );
}

QString Nepomuk::KMetaData::Resource::preflabelUri()
{
    return "http://semanticdesktop.org/ontologies/2007/03/31/nao#preflabel";
}

QList<Nepomuk::KMetaData::Resource> Nepomuk::KMetaData::Resource::getIsRelateds() const
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    return convertResourceList<Resource>( getProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#isRelated" ).toResourceList() );
}

void Nepomuk::KMetaData::Resource::setIsRelateds( const QList<Nepomuk::KMetaData::Resource>& value )
{
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#isRelated", Variant( value ) );
}

void Nepomuk::KMetaData::Resource::addIsRelated( const Nepomuk::KMetaData::Resource& value )
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    Variant v = getProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#isRelated" );
    v.append( Resource( value ) );
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#isRelated", v );
}

QString Nepomuk::KMetaData::Resource::isRelatedUri()
{
    return "http://semanticdesktop.org/ontologies/2007/03/31/nao#isRelated";
}

QStringList Nepomuk::KMetaData::Resource::getLabels() const
{
    return getProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#altLabel" ).toStringList();
}

void Nepomuk::KMetaData::Resource::setLabels( const QStringList& value )
{
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#altLabel", Variant( value ) );
}

void Nepomuk::KMetaData::Resource::addLabel( const QString& value )
{
    Variant v = getProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#altLabel" );
    v.append( value );
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#altLabel", v );
}

QString Nepomuk::KMetaData::Resource::LabelUri()
{
    return "http://semanticdesktop.org/ontologies/2007/03/31/nao#altLabel";
}

QList<Nepomuk::KMetaData::Resource> Nepomuk::KMetaData::Resource::getIsTopicOfs() const
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    return convertResourceList<Resource>( getProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#isTopicOf" ).toResourceList() );
}

void Nepomuk::KMetaData::Resource::setIsTopicOfs( const QList<Nepomuk::KMetaData::Resource>& value )
{
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#isTopicOf", Variant( value ) );
}

void Nepomuk::KMetaData::Resource::addIsTopicOf( const Nepomuk::KMetaData::Resource& value )
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    Variant v = getProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#isTopicOf" );
    v.append( Resource( value ) );
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#isTopicOf", v );
}

QString Nepomuk::KMetaData::Resource::isTopicOfUri()
{
    return "http://semanticdesktop.org/ontologies/2007/03/31/nao#isTopicOf";
}

QList<Nepomuk::KMetaData::Resource> Nepomuk::KMetaData::Resource::getAnnotations() const
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    return convertResourceList<Resource>( getProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#annotation" ).toResourceList() );
}

void Nepomuk::KMetaData::Resource::setAnnotations( const QList<Nepomuk::KMetaData::Resource>& value )
{
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#annotation", Variant( value ) );
}

void Nepomuk::KMetaData::Resource::addAnnotation( const Nepomuk::KMetaData::Resource& value )
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    Variant v = getProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#annotation" );
    v.append( Resource( value ) );
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#annotation", v );
}

QString Nepomuk::KMetaData::Resource::annotationUri()
{
    return "http://semanticdesktop.org/ontologies/2007/03/31/nao#annotation";
}

QString Nepomuk::KMetaData::Resource::getComment() const
{
    return getProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasComment" ).value<QString>();
}

void Nepomuk::KMetaData::Resource::setComment( const QString& value )
{
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasComment", Variant( value ) );
}

QString Nepomuk::KMetaData::Resource::CommentUri()
{
    return "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasComment";
}

QList<Nepomuk::KMetaData::Tag> Nepomuk::KMetaData::Resource::getTags() const
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    return convertResourceList<Tag>( getProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasTag" ).toResourceList() );
}

void Nepomuk::KMetaData::Resource::setTags( const QList<Nepomuk::KMetaData::Tag>& value )
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

void Nepomuk::KMetaData::Resource::addTag( const Nepomuk::KMetaData::Tag& value )
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    Variant v = getProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasTag" );
    v.append( Resource( value ) );
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasTag", v );
}

QString Nepomuk::KMetaData::Resource::TagUri()
{
    return "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasTag";
}

QList<Nepomuk::KMetaData::Resource> Nepomuk::KMetaData::Resource::getTopics() const
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    return convertResourceList<Resource>( getProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasTopic" ).toResourceList() );
}

void Nepomuk::KMetaData::Resource::setTopics( const QList<Nepomuk::KMetaData::Resource>& value )
{
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasTopic", Variant( value ) );
}

void Nepomuk::KMetaData::Resource::addTopic( const Nepomuk::KMetaData::Resource& value )
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    Variant v = getProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasTopic" );
    v.append( Resource( value ) );
    setProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasTopic", v );
}

QString Nepomuk::KMetaData::Resource::TopicUri()
{
    return "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasTopic";
}

QList<Nepomuk::KMetaData::Resource> Nepomuk::KMetaData::Resource::annotationOf() const
{
    return convertResourceList<Resource>( ResourceManager::instance()->allResourcesWithProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#annotation", *this ) );
}

QList<Nepomuk::KMetaData::Resource> Nepomuk::KMetaData::Resource::TopicOf() const
{
    return convertResourceList<Resource>( ResourceManager::instance()->allResourcesWithProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasTopic", *this ) );
}

QList<Nepomuk::KMetaData::Resource> Nepomuk::KMetaData::Resource::isRelatedOf() const
{
    return convertResourceList<Resource>( ResourceManager::instance()->allResourcesWithProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#isRelated", *this ) );
}

QList<Nepomuk::KMetaData::Resource> Nepomuk::KMetaData::Resource::isTopicOfOf() const
{
    return convertResourceList<Resource>( ResourceManager::instance()->allResourcesWithProperty( "http://semanticdesktop.org/ontologies/2007/03/31/nao#isTopicOf", *this ) );
}

QList<Nepomuk::KMetaData::Resource> Nepomuk::KMetaData::Resource::allResources()
{
    return Nepomuk::KMetaData::convertResourceList<Resource>( ResourceManager::instance()->allResourcesOfType( "http://www.w3.org/2000/01/rdf-schema#Resource" ) );
}



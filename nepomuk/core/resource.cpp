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

#include "resource.h"
#include "resourcedata.h"
#include "resourcemanager.h"
#include "resourcemanager_p.h"
#include "tools.h"
#include "tag.h"
#include "pimo.h"
#include "thing.h"

#include <klocale.h>
#include <kdebug.h>

#include <Soprano/Vocabulary/NAO>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/RDFS>

#include <kmimetype.h>


Nepomuk::Resource::Resource()
{
    m_data = ResourceManager::instance()->d->data( QUrl(), QUrl() );
    m_data->ref();
}


Nepomuk::Resource::Resource( ResourceManager* manager )
{
    m_data = manager->d->data( QUrl(), QUrl() );
    m_data->ref();
}


Nepomuk::Resource::Resource( const Nepomuk::Resource& res )
{
    m_data = res.m_data;
    if ( m_data )
        m_data->ref();
}


Nepomuk::Resource::Resource( const QString& uri, const QUrl& type )
{
    m_data = ResourceManager::instance()->d->data( uri, type );
    if ( m_data )
        m_data->ref();
}


Nepomuk::Resource::Resource( const QString& uri, const QUrl& type, ResourceManager* manager )
{
    m_data = manager->d->data( uri, type );
    if ( m_data )
        m_data->ref();
}


Nepomuk::Resource::Resource( const QString& uri, const QString& type )
{
    m_data = ResourceManager::instance()->d->data( uri, type );
    if ( m_data )
        m_data->ref();
}


Nepomuk::Resource::Resource( const QUrl& uri, const QUrl& type )
{
    m_data = ResourceManager::instance()->d->data( uri, type );
    if ( m_data )
        m_data->ref();
}


Nepomuk::Resource::Resource( const QUrl& uri, const QUrl& type, ResourceManager* manager )
{
    m_data = manager->d->data( uri, type );
    if ( m_data )
        m_data->ref();
}


Nepomuk::Resource::Resource( Nepomuk::ResourceData* data )
{
    m_data = data;
    if ( m_data )
        data->ref();
}


Nepomuk::Resource::~Resource()
{
    // FIXME: ResourceData instances having a proxy also need to be deleted, maybe extend deref
    if( m_data &&
        m_data->deref() == 0 &&
        ( !m_data->isValid() || m_data->rm()->dataCacheFull() ) ) {
        m_data->deleteData();
    }
}


Nepomuk::Resource& Nepomuk::Resource::operator=( const Resource& res )
{
    if( m_data != res.m_data ) {
        if ( m_data && m_data->deref() == 0 && !m_data->isValid() ) {
            m_data->deleteData();
        }
        m_data = res.m_data;
        if ( m_data )
            m_data->ref();
    }

    return *this;
}


Nepomuk::Resource& Nepomuk::Resource::operator=( const QUrl& res )
{
    return operator=( Resource( res ) );
}


Nepomuk::ResourceManager* Nepomuk::Resource::manager() const
{
    return m_data->rm()->m_manager;
}


QString Nepomuk::Resource::uri() const
{
    return resourceUri().toString();
}


QUrl Nepomuk::Resource::resourceUri() const
{
    if ( m_data ) {
        m_data->determineUri();
        return m_data->uri();
    }
    else {
        return QUrl();
    }
}


QString Nepomuk::Resource::type() const
{
    return resourceType().toString();
}


QUrl Nepomuk::Resource::resourceType() const
{
    if ( m_data ) {
        return m_data->type();
    }
    else {
        return QUrl();
    }
}


QList<QUrl> Nepomuk::Resource::types() const
{
    if ( m_data ) {
        return m_data->allTypes();
    }
    else {
        return QList<QUrl>();
    }
}


void Nepomuk::Resource::setTypes( const QList<QUrl>& types )
{
    if ( m_data )
        m_data->setTypes( types );
}


void Nepomuk::Resource::addType( const QUrl& type )
{
    if ( m_data )
        setTypes( types() << type );
}


bool Nepomuk::Resource::hasType( const QUrl& typeUri ) const
{
    return m_data ? m_data->hasType( typeUri ) : false;
}


QString Nepomuk::Resource::className() const
{
    return resourceType().toString().section( QRegExp( "[#:]" ), -1 );
}


QHash<QUrl, Nepomuk::Variant> Nepomuk::Resource::properties() const
{
    if ( m_data ) {
        return m_data->allProperties();
    }
    else {
        return QHash<QUrl, Nepomuk::Variant>();
    }
}


QHash<QString, Nepomuk::Variant> Nepomuk::Resource::allProperties() const
{
    QHash<QString, Nepomuk::Variant> pl;
    QHash<QUrl, Nepomuk::Variant> p = properties();
    QHash<QUrl, Nepomuk::Variant>::const_iterator end = p.constEnd();
    for ( QHash<QUrl, Nepomuk::Variant>::const_iterator it = p.constBegin();
          it != end; ++it ) {
        pl.insert( it.key().toString(), it.value() );
    }
    return pl;
}


bool Nepomuk::Resource::hasProperty( const QUrl& uri ) const
{
    return m_data ? m_data->hasProperty( uri ) : false;
}


bool Nepomuk::Resource::hasProperty( const QString& uri ) const
{
    return hasProperty( QUrl( uri ) );
}


Nepomuk::Variant Nepomuk::Resource::property( const QString& uri ) const
{
    return property( QUrl( uri ) );
}


Nepomuk::Variant Nepomuk::Resource::property( const QUrl& uri ) const
{
    if ( m_data ) {
        return m_data->property( uri );
    }
    else {
        return Variant();
    }
}


void Nepomuk::Resource::setProperty( const QString& uri, const Nepomuk::Variant& value )
{
    setProperty( QUrl( uri ), value );
}


void Nepomuk::Resource::addProperty( const QUrl& uri, const Variant& value )
{
    Variant v = property( uri );
    v.append( value );
    setProperty( uri, v );
}


void Nepomuk::Resource::setProperty( const QUrl& uri, const Nepomuk::Variant& value )
{
    if ( m_data ) {
        m_data->setProperty( uri, value );
    }
}


void Nepomuk::Resource::removeProperty( const QString& uri )
{
    removeProperty( QUrl( uri ) );
}


void Nepomuk::Resource::removeProperty( const QUrl& uri )
{
    if ( m_data ) {
        m_data->removeProperty( uri );
    }
}


void Nepomuk::Resource::removeProperty( const QUrl& uri, const Variant& value )
{
    if ( m_data ) {
        QList<Variant> vl = property( uri ).toVariantList();
        foreach( const Variant& v, value.toVariantList() ) {
            vl.removeAll( v );
        }
        setProperty( uri, Variant( vl ) );
    }
}


void Nepomuk::Resource::remove()
{
    if ( m_data ) {
        m_data->remove();
    }
}


bool Nepomuk::Resource::exists() const
{
    return m_data ? m_data->exists() : false;
}


bool Nepomuk::Resource::isValid() const
{
    return m_data ? m_data->isValid() : false;
}


QString Nepomuk::Resource::genericLabel() const
{
    QString label = this->label();
    if ( label.isEmpty() ) {
        label = property( Soprano::Vocabulary::RDFS::label() ).toString();

        if ( label.isEmpty() ) {
            label = property( Soprano::Vocabulary::NAO::identifier() ).toString();

            if ( label.isEmpty() ) {
                label = property( Soprano::Vocabulary::Xesam::name() ).toString();

                if ( label.isEmpty() ) {
                    label = property( Soprano::Vocabulary::Xesam::url() ).toString().section( '/', -1 );

                    if ( label.isEmpty() ) {
                        QList<Resource> go = property( Vocabulary::PIMO::groundingOccurrence() ).toResourceList();
                        if( !go.isEmpty() ) {
                            label = go.first().genericLabel();
                            if( label == go.first().resourceUri().toString() ) {
                                label.clear();
                            }
                        }

                        if ( label.isEmpty() ) {
                            // ugly fallback
                            label = resourceUri().toString();
                        }
                    }
                }
            }
        }
    }

    return label;
}


QString Nepomuk::Resource::genericDescription() const
{
    QString s = property( Soprano::Vocabulary::NAO::description() ).toString();
    if ( !s.isEmpty() ) {
        return s;
    }

    s = property( Soprano::Vocabulary::Xesam::summary() ).toString();
    if ( !s.isEmpty() ) {
        return s;
    }

    s = property( Soprano::Vocabulary::Xesam::description() ).toString();
    if ( !s.isEmpty() ) {
        return s;
    }

    s = property( Soprano::Vocabulary::Xesam::asText() ).toString();
    if ( !s.isEmpty() ) {
        return s;
    }

    s = property( Soprano::Vocabulary::RDFS::comment() ).toString();

    return s;
}


QString Nepomuk::Resource::genericIcon() const
{
    // FIXME: support resource symbols
    Variant symbol = property( Soprano::Vocabulary::NAO::hasSymbol() );
    if ( symbol.isString() ) {
        return symbol.toString();
    }

    // strigi mimetypes are sadly not very reliable, I keep the code here for future use
//     QString mimeType = property( Soprano::Vocabulary::Xesam::mimeType() ).toString();
//     if ( !mimeType.isEmpty() ) {
//         if ( KMimeType::Ptr m = KMimeType::mimeType( mimeType ) ) {
//             return m->iconName();
//         }
//     }

    if ( hasType( Soprano::Vocabulary::Xesam::File() ) ||
         resourceUri().scheme() == "file" ) {
        return KMimeType::iconNameForUrl( resourceUri() );
    }

    return QString();
}


Nepomuk::Thing Nepomuk::Resource::pimoThing()
{
    if( m_data ) {
        return m_data->pimoThing();
    }
    else {
        return Thing();
    }
}


bool Nepomuk::Resource::operator==( const Resource& other ) const
{
    if( this == &other )
        return true;

    if( this->m_data == other.m_data )
        return true;

    if ( !m_data || !other.m_data ) {
        return false;
    }


    m_data->determineUri();
    other.m_data->determineUri();
    return resourceUri() == other.resourceUri();
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


QString Nepomuk::Resource::description() const
{
    return ( property( Soprano::Vocabulary::NAO::description() ).toStringList() << QString() ).first();
}


void Nepomuk::Resource::setDescription( const QString& value )
{
    setProperty( Soprano::Vocabulary::NAO::description(), Variant( value ) );
}


QString Nepomuk::Resource::descriptionUri()
{
    return Soprano::Vocabulary::NAO::description().toString();
}


QStringList Nepomuk::Resource::identifiers() const
{
    return property( Soprano::Vocabulary::NAO::identifier() ).toStringList();
}


void Nepomuk::Resource::setIdentifiers( const QStringList& value )
{
    setProperty( Soprano::Vocabulary::NAO::identifier(), Variant( value ) );
}


void Nepomuk::Resource::addIdentifier( const QString& value )
{
    Variant v = property( Soprano::Vocabulary::NAO::identifier() );
    v.append( value );
    setProperty( Soprano::Vocabulary::NAO::identifier(), v );
}


QString Nepomuk::Resource::identifierUri()
{
    return Soprano::Vocabulary::NAO::identifier().toString();
}


QStringList Nepomuk::Resource::altLabels() const
{
    return property( Soprano::Vocabulary::NAO::altLabel() ).toStringList();
}


void Nepomuk::Resource::setAltLabels( const QStringList& value )
{
    setProperty( Soprano::Vocabulary::NAO::altLabel(), Variant( value ) );
}


void Nepomuk::Resource::addAltLabel( const QString& value )
{
    Variant v = property( Soprano::Vocabulary::NAO::altLabel() );
    v.append( value );
    setProperty( Soprano::Vocabulary::NAO::altLabel(), v );
}


QString Nepomuk::Resource::altLabelUri()
{
    return Soprano::Vocabulary::NAO::altLabel().toString();
}


QList<Nepomuk::Resource> Nepomuk::Resource::annotations() const
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    return convertResourceList<Resource>( property( Soprano::Vocabulary::NAO::annotation() ).toResourceList() );
}


void Nepomuk::Resource::setAnnotations( const QList<Nepomuk::Resource>& value )
{
    setProperty( Soprano::Vocabulary::NAO::annotation(), Variant( value ) );
}


void Nepomuk::Resource::addAnnotation( const Nepomuk::Resource& value )
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    Variant v = property( Soprano::Vocabulary::NAO::annotation() );
    v.append( Resource( value ) );
    setProperty( Soprano::Vocabulary::NAO::annotation(), v );
}


QString Nepomuk::Resource::annotationUri()
{
    return Soprano::Vocabulary::NAO::annotation().toString();
}


QList<Nepomuk::Tag> Nepomuk::Resource::tags() const
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    return convertResourceList<Tag>( property( Soprano::Vocabulary::NAO::hasTag() ).toResourceList() );
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
    setProperty( Soprano::Vocabulary::NAO::hasTag(), Variant( l ) );
}


void Nepomuk::Resource::addTag( const Nepomuk::Tag& value )
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    Variant v = property( Soprano::Vocabulary::NAO::hasTag() );
    v.append( Resource( value ) );
    setProperty( Soprano::Vocabulary::NAO::hasTag(), v );
}


QString Nepomuk::Resource::tagUri()
{
    return Soprano::Vocabulary::NAO::hasTag().toString();
}


QList<Nepomuk::Resource> Nepomuk::Resource::topics() const
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    return convertResourceList<Resource>( property( Soprano::Vocabulary::NAO::hasTopic() ).toResourceList() );
}


void Nepomuk::Resource::setTopics( const QList<Nepomuk::Resource>& value )
{
    setProperty( Soprano::Vocabulary::NAO::hasTopic(), Variant( value ) );
}


void Nepomuk::Resource::addTopic( const Nepomuk::Resource& value )
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    Variant v = property( Soprano::Vocabulary::NAO::hasTopic() );
    v.append( Resource( value ) );
    setProperty( Soprano::Vocabulary::NAO::hasTopic(), v );
}


QString Nepomuk::Resource::topicUri()
{
    return Soprano::Vocabulary::NAO::hasTopic().toString();
}


QList<Nepomuk::Resource> Nepomuk::Resource::isTopicOfs() const
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    return convertResourceList<Resource>( property( Soprano::Vocabulary::NAO::isTopicOf() ).toResourceList() );
}


void Nepomuk::Resource::setIsTopicOfs( const QList<Nepomuk::Resource>& value )
{
    setProperty( Soprano::Vocabulary::NAO::isTopicOf(), Variant( value ) );
}


void Nepomuk::Resource::addIsTopicOf( const Nepomuk::Resource& value )
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    Variant v = property( Soprano::Vocabulary::NAO::isTopicOf() );
    v.append( Resource( value ) );
    setProperty( Soprano::Vocabulary::NAO::isTopicOf(), v );
}


QString Nepomuk::Resource::isTopicOfUri()
{
    return Soprano::Vocabulary::NAO::isTopicOf().toString();
}


QList<Nepomuk::Resource> Nepomuk::Resource::isRelateds() const
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    return convertResourceList<Resource>( property( Soprano::Vocabulary::NAO::isRelated() ).toResourceList() );
}


void Nepomuk::Resource::setIsRelateds( const QList<Nepomuk::Resource>& value )
{
    setProperty( Soprano::Vocabulary::NAO::isRelated(), Variant( value ) );
}


void Nepomuk::Resource::addIsRelated( const Nepomuk::Resource& value )
{
    // We always store all Resource types as plain Resource objects.
    // It does not introduce any overhead (due to the implicit sharing of
    // the data and has the advantage that we can mix setProperty calls
    // with the special Resource subclass methods.
    // More importantly Resource loads the data as Resource objects anyway.
    Variant v = property( Soprano::Vocabulary::NAO::isRelated() );
    v.append( Resource( value ) );
    setProperty( Soprano::Vocabulary::NAO::isRelated(), v );
}


QString Nepomuk::Resource::isRelatedUri()
{
    return Soprano::Vocabulary::NAO::isRelated().toString();
}


QString Nepomuk::Resource::label() const
{
    return ( property( Soprano::Vocabulary::NAO::prefLabel() ).toStringList() << QString() ).first();
}


void Nepomuk::Resource::setLabel( const QString& value )
{
    setProperty( Soprano::Vocabulary::NAO::prefLabel(), Variant( value ) );
}


QString Nepomuk::Resource::labelUri()
{
    return Soprano::Vocabulary::NAO::prefLabel().toString();
}


quint32 Nepomuk::Resource::rating() const
{
    return ( property( Soprano::Vocabulary::NAO::numericRating() ).toUnsignedIntList() << 0 ).first();
}


void Nepomuk::Resource::setRating( const quint32& value )
{
    setProperty( Soprano::Vocabulary::NAO::numericRating(), Variant( value ) );
}


QString Nepomuk::Resource::ratingUri()
{
    return Soprano::Vocabulary::NAO::numericRating().toString();
}


QStringList Nepomuk::Resource::symbols() const
{
    return property( Soprano::Vocabulary::NAO::hasSymbol() ).toStringList();
}


void Nepomuk::Resource::setSymbols( const QStringList& value )
{
    setProperty( Soprano::Vocabulary::NAO::hasSymbol(), Variant( value ) );
}


void Nepomuk::Resource::addSymbol( const QString& value )
{
    Variant v = property( Soprano::Vocabulary::NAO::hasSymbol() );
    v.append( value );
    setProperty( Soprano::Vocabulary::NAO::hasSymbol(), v );
}


QString Nepomuk::Resource::symbolUri()
{
    return Soprano::Vocabulary::NAO::hasSymbol().toString();
}


QList<Nepomuk::Resource> Nepomuk::Resource::annotationOf() const
{
    return convertResourceList<Resource>( manager()->allResourcesWithProperty( Soprano::Vocabulary::NAO::annotation(), *this ) );
}


QList<Nepomuk::Resource> Nepomuk::Resource::isRelatedOf() const
{
    return convertResourceList<Resource>( manager()->allResourcesWithProperty( Soprano::Vocabulary::NAO::isRelated(), *this ) );
}


// static
QList<Nepomuk::Resource> Nepomuk::Resource::allResources()
{
    return Nepomuk::convertResourceList<Resource>( ResourceManager::instance()->allResourcesOfType( Soprano::Vocabulary::RDFS::Resource() ) );
}

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

#include "ontologyparser.h"

#include "property.h"
#include "resourceclass.h"

#include <soprano/soprano.h>

#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtCore/QFile>
#include <QtCore/QHash>
#include <QtCore/QDebug>
#include <QtCore/QDir>


extern bool quiet;


class OntologyParser::Private
{
public:
    Private() {
        // default parent class
        resources.insert( Soprano::Vocabulary::RDFS::Resource(),
                          ResourceClass( Soprano::Vocabulary::RDFS::Resource() ) );

        // build xsd -> Qt type map
        xmlSchemaTypes.insert( Soprano::Vocabulary::XMLSchema::integer(), "qint64" );
        xmlSchemaTypes.insert( Soprano::Vocabulary::XMLSchema::negativeInteger(), "qint64" );
        xmlSchemaTypes.insert( Soprano::Vocabulary::XMLSchema::nonNegativeInteger(), "quint64" );
        xmlSchemaTypes.insert( Soprano::Vocabulary::XMLSchema::xsdLong(), "qint64" );
        xmlSchemaTypes.insert( Soprano::Vocabulary::XMLSchema::unsignedLong(), "quint64" );
        xmlSchemaTypes.insert( Soprano::Vocabulary::XMLSchema::xsdInt(), "qint32" );
        xmlSchemaTypes.insert( Soprano::Vocabulary::XMLSchema::unsignedInt(), "quint32" );
        xmlSchemaTypes.insert( Soprano::Vocabulary::XMLSchema::xsdShort(), "qint16" );
        xmlSchemaTypes.insert( Soprano::Vocabulary::XMLSchema::unsignedShort(), "quint16" );
        xmlSchemaTypes.insert( Soprano::Vocabulary::XMLSchema::xsdFloat(), "double" );
        xmlSchemaTypes.insert( Soprano::Vocabulary::XMLSchema::xsdDouble(), "double" );
        xmlSchemaTypes.insert( Soprano::Vocabulary::XMLSchema::boolean(), "bool" );
        xmlSchemaTypes.insert( Soprano::Vocabulary::XMLSchema::date(), "QDate" );
        xmlSchemaTypes.insert( Soprano::Vocabulary::XMLSchema::time(), "QTime" );
        xmlSchemaTypes.insert( Soprano::Vocabulary::XMLSchema::dateTime(), "QDateTime" );
        xmlSchemaTypes.insert( Soprano::Vocabulary::XMLSchema::string(), "QString" );
    }

    // the classes to be generated as set on the command line
    // if empty, all classes are generated
    QStringList classesToGenerate;

    // the optional visibility string. If not-empty generated classes will be exported via
    // <visibility>_EXPORT by including the header <visibility>_export.h.
    QString visibility;

    QMap<QUrl, ResourceClass> resources;
    QMap<QUrl, Property> properties;
    QMap<QUrl, QString> comments;
    const Soprano::Parser* rdfParser;

    QHash<QUrl, QString> xmlSchemaTypes;

    ResourceClass* getClass( const QUrl& uri ) {
        ResourceClass& r = resources[uri];
        r.setUri( uri );
        if ( !r.parentClass(false) ) {
            r.setParentResource( &resources[Soprano::Vocabulary::RDFS::Resource()] );
        }
        return &r;
    }

    Property* getProperty( const QUrl& uri ) {
        Property& p = properties[uri];
        p.setUri( uri );
        return &p;
    }
};


OntologyParser::OntologyParser()
{
    d = new Private;
    d->rdfParser = 0;
}


OntologyParser::~OntologyParser()
{
    delete d;
}


bool OntologyParser::parse( const QString& filename, const QString& serializationMimeType )
{
    Soprano::RdfSerialization serialization = Soprano::SerializationUnknown;
    if( !serializationMimeType.isEmpty() ) {
        serialization = Soprano::mimeTypeToSerialization( serializationMimeType );
    }
    else if ( filename.endsWith( "trig" ) ) {
        serialization = Soprano::SerializationTrig;
    }
    else {
        serialization = Soprano::SerializationRdfXml;
    }

    if ( !( d->rdfParser = Soprano::PluginManager::instance()->discoverParserForSerialization( serialization, serializationMimeType ) ) ) {
        return false;
    }

    if ( !quiet )
        qDebug() << "(OntologyParser) Parsing " << filename << endl;

    Soprano::StatementIterator it = d->rdfParser->parseFile( filename,
                                                             QUrl("http://org.kde.nepomuk/dummybaseuri"),
                                                             serialization,
                                                             serializationMimeType );
    bool success = true;

    while( it.next() ) {
        const Soprano::Statement& s = *it;

        if( s.predicate().uri() == Soprano::Vocabulary::RDFS::subClassOf() ) {
            ResourceClass* rc = d->getClass( s.subject().uri().toString() );
            rc->setParentResource( d->getClass( s.object().uri().toString() ) );
            rc->addParentResource( d->getClass( s.object().uri().toString() ) );
            rc->setGenerateClass( true );
        }
        else if( s.predicate().uri() == Soprano::Vocabulary::RDF::type() ) {
            if( s.object().uri().toString().endsWith( "#Class" ) )
                d->getClass( s.subject().uri().toString() )->setGenerateClass( true );
        }
        else if( s.predicate().uri() == Soprano::Vocabulary::RDFS::domain() ) {
            ResourceClass* rc = d->getClass( s.object().uri().toString() );
            Property* p = d->getProperty( s.subject().uri().toString() );
            p->setDomain( rc );
            if ( !rc->allProperties().contains( p ) )
                rc->addProperty( p );
            rc->setGenerateClass( true );
        }
        else if( s.predicate().uri() == Soprano::Vocabulary::RDFS::range() ) {
            Property* prop = d->getProperty(s.subject().uri().toString());
            QUrl type = s.object().uri();
            if( type.toString().contains( "XMLSchema" ) ) {
                prop->setLiteralRange( d->xmlSchemaTypes[type] );
            }
            else if( type == Soprano::Vocabulary::RDFS::Literal() ) {
                prop->setLiteralRange( "QString" );
            }
            else {
                prop->setRange( d->getClass(s.object().uri()) );
            }
        }
        else if( s.predicate().uri() == Soprano::Vocabulary::NRL::maxCardinality() ||
                 s.predicate().uri() == Soprano::Vocabulary::NRL::cardinality() ) {
            d->getProperty(s.subject().uri())->setIsList( s.object().literal().toInt() > 1 );
        }
        else if( s.predicate().uri() == Soprano::Vocabulary::RDFS::comment() ) {
            d->comments[s.subject().uri()] = s.object().literal().toString();
        }
        else if ( s.predicate().uri() == Soprano::Vocabulary::NRL::inverseProperty() ) {
            d->getProperty(s.subject().uri())->setInverseProperty( d->getProperty(s.object().uri()) );
            d->getProperty(s.object().uri())->setInverseProperty( d->getProperty(s.subject().uri()) );
        }
    }

    // determine the reverse properties
    for( QMap<QUrl, Property>::iterator propIt = d->properties.begin();
         propIt != d->properties.end(); ++propIt ) {
        Property& p = propIt.value();
        if( p.range() ) {
            if ( !quiet )
                qDebug() << "Setting reverse property " << p.uri() << " on type " << p.range()->uri() << endl;
            if ( !p.range()->allReverseProperties().contains( &p ) )
                p.range()->addReverseProperty( &p );
        }
        if ( !p.domain() ) {
            p.setDomain( d->getClass(Soprano::Vocabulary::RDFS::Resource()) );
        }

        Q_ASSERT( d->properties.count( propIt.key() ) == 1 );
    }

    // now assign the comments to resources and properties
    QMapIterator<QUrl, QString> commentsIt( d->comments );
    while( commentsIt.hasNext() ) {
        commentsIt.next();
        if( d->resources.contains( commentsIt.key() ) )
            d->resources[commentsIt.key()].setComment( commentsIt.value() );
        else if( d->properties.contains( commentsIt.key() ) )
            d->properties[commentsIt.key()].setComment( commentsIt.value() );
    }

    // testing stuff
    for( QMap<QUrl, ResourceClass>::iterator it = d->resources.begin();
         it != d->resources.end(); ++it ) {
        if( !it->parentClass(false) ) {
            it->setParentResource( d->getClass(Soprano::Vocabulary::RDFS::Resource()) );
        }
        if ( !quiet )
            qDebug() << "Resource: " << (*it).name()
                     << "[" << (*it).uri() << "]"
                     << " (->" << (*it).parentClass(false)->name() << ")"
                     << ( (*it).generateClass() ? " (will be generated)" : " (will not be generated)" )
                     << endl;

        Q_ASSERT( d->resources.count( it.key() ) == 1 );

        QListIterator<const Property*> propIt( (*it).allProperties() );
        while( propIt.hasNext() ) {
            const Property* p = propIt.next();
            if ( !quiet )
                qDebug() << "          " << p->uri() << " (->" << p->typeString() << ")" << ( p->isList() ? QString("+") : QString("1") ) << endl;
        }
    }

    return success;
}


QList<ResourceClass*> OntologyParser::parsedClasses() const
{
    QList<ResourceClass*> rl;
    for( QMap<QUrl, ResourceClass>::iterator it = d->resources.begin();
         it != d->resources.end(); ++it )
        rl << &(*it);
    return rl;
}

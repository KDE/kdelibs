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
#include "resourceclass.h"
#include "resourcetemplate.h"

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
        resources.insert( "http://www.w3.org/2000/01/rdf-schema#Resource",
                          ResourceClass( "http://www.w3.org/2000/01/rdf-schema#Resource" ) );
    }

    QMap<QString, ResourceClass> resources;
    QMap<QString, Property> properties;
    QMap<QString, QString> comments;
    const Soprano::Parser* rdfParser;

    QHash<QString, QString> namespaceAbbr;

    QString ensureNS( const QString& uri ) {
        // check if we have a NS abbrev
        QString ns = uri.left( uri.indexOf( ":" ) );
        if( namespaceAbbr.contains( ns ) )
            return namespaceAbbr[ns] + uri.mid( ns.length()+1 );
        else
            return uri;
    }

    ResourceClass& getResource( const QString& uri ) {
        ResourceClass& r = resources[ensureNS(uri)];
        r.uri = ensureNS(uri);
        if ( !r.parent ) {
            r.parent = &resources["http://www.w3.org/2000/01/rdf-schema#Resource"];
        }
        return r;
    }

    Property& getProperty( const QString& uri ) {
        Property& p = properties[ensureNS(uri)];
        p.uri = ensureNS(uri);
        return p;
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


bool OntologyParser::assignTemplates( const QStringList& templates )
{
    // FIXME: do an actual class name mapping by parsing the class
    foreach( const QString &tf, templates ) {
        QString filename = QFileInfo( tf ).fileName();
        for( QMap<QString, ResourceClass>::iterator it = d->resources.begin();
             it != d->resources.end(); ++it ) {
            // we use startsWith() for a hackish handling of such suffixes as ".in"
            if( filename.startsWith( it.value().headerName() ) ) {
                if ( !quiet )
                    qDebug() << "Using header template file " << tf << " for class " << it.value().name();
                it.value().headerTemplateFilePath = tf;
            }
            else if( filename.startsWith( it.value().sourceName() ) ) {
                if ( !quiet )
                    qDebug() << "Using source template file " << tf << " for class " << it.value().name();
                it.value().sourceTemplateFilePath = tf;
            }
        }
    }

    return true;
}


bool OntologyParser::parse( const QString& filename )
{
    Soprano::RdfSerialization serialization;
    if ( filename.endsWith( "trig" ) ) {
        serialization = Soprano::SerializationTrig;
    }
    else {
        serialization = Soprano::SerializationRdfXml;
    }

    if ( !( d->rdfParser = Soprano::PluginManager::instance()->discoverParserForSerialization( serialization ) ) ) {
        return false;
    }

    if ( !quiet )
        qDebug() << "(OntologyParser) Parsing " << filename << endl;

    // get the namespaces the hacky way
    QFile f( filename );
    if( f.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
        QString s = QTextStream( &f ).readAll();
        QRegExp nsr( "xmlns:(\\S*)=\"(\\S*\\#)\"" );
        int pos = 0;
        while( ( pos = s.indexOf( nsr, pos+1 ) ) > 0 ) {
            if ( !quiet )
                qDebug() << "Found namespace abbreviation: " << nsr.cap(1) << "->" << nsr.cap(2) << endl;
            d->namespaceAbbr.insert( nsr.cap(1), nsr.cap(2) );
        }
    }

    // FIXME: the serialization should be somehow specified
    Soprano::StatementIterator it = d->rdfParser->parseFile( filename,
                                                             QUrl("http://org.kde.nepomuk/dummybaseuri"),
                                                             serialization );
    bool success = true;

    while( it.next() ) {
        const Soprano::Statement& s = *it;

        if( s.predicate().uri().toString().endsWith( "#subClassOf" ) ) {
            ResourceClass& rc = d->getResource( s.subject().uri().toString() );
            rc.parent = &d->getResource( s.object().uri().toString() );
            rc.allParents.append( &d->getResource( s.object().uri().toString() ) );
            rc.generate = true;
        }
        else if( s.predicate().uri().toString().endsWith( "#type" ) ) {
            if( s.object().uri().toString().endsWith( "#Class" ) )
                d->getResource( s.subject().uri().toString() ).generate = true;
        }
        else if( s.predicate().uri().toString().endsWith( "#domain" ) ) {
            ResourceClass& rc = d->getResource( s.object().uri().toString() );
            Property& p = d->getProperty( s.subject().uri().toString() );
            p.domain = &rc;
            if ( !rc.properties.contains( &p ) )
                rc.properties.append( &p );
            rc.generate = true;
        }
        else if( s.predicate().uri().toString().endsWith( "#range" ) ) {
            d->getProperty(s.subject().uri().toString()).type = d->ensureNS(s.object().uri().toString());
        }
        else if( s.predicate().uri().toString().endsWith( "#maxCardinality" ) ||
                 s.predicate().uri().toString().endsWith( "#cardinality" ) ) {
            d->getProperty(s.subject().uri().toString()).list = ( s.object().literal().toInt() > 1 );
        }
        else if( s.predicate().uri().toString().endsWith( "#comment" ) ) {
            d->comments[d->ensureNS(s.subject().uri().toString())] = s.object().literal().toString();
        }
        else if ( s.predicate().uri().toString().endsWith("inverseProperty") ) {
            d->getProperty(s.subject().uri().toString()).inverse = &d->getProperty(s.object().uri().toString());
            d->getProperty(s.object().uri().toString()).inverse = &d->getProperty(s.subject().uri().toString());
        }
    }

    // determine the reverse properties
    for( QMap<QString, Property>::iterator propIt = d->properties.begin();
         propIt != d->properties.end(); ++propIt ) {
        Property& p = propIt.value();
        if( d->resources.contains( p.type ) ) {
            if ( !quiet )
                qDebug() << "Setting reverse property " << p.uri << " on type " << p.type << endl;
            if ( !d->resources[p.type].reverseProperties.contains( &p ) )
                d->resources[p.type].reverseProperties.append( &p );
        }
        if ( !p.domain ) {
            p.domain = &d->resources["http://www.w3.org/2000/01/rdf-schema#Resource"];
        }

        Q_ASSERT( d->properties.count( propIt.key() ) == 1 );
    }

    // now assign the comments to resources and properties
    QMapIterator<QString, QString> commentsIt( d->comments );
    while( commentsIt.hasNext() ) {
        commentsIt.next();
        if( d->resources.contains( commentsIt.key() ) )
            d->resources[commentsIt.key()].comment = commentsIt.value();
        else if( d->properties.contains( commentsIt.key() ) )
            d->properties[commentsIt.key()].comment = commentsIt.value();
    }

    // testing stuff
    for( QMap<QString, ResourceClass>::iterator it = d->resources.begin();
         it != d->resources.end(); ++it ) {
        if( !it->parent ) {
            it->parent = &d->resources["http://www.w3.org/2000/01/rdf-schema#Resource"];
        }
        if ( !quiet )
            qDebug() << "Resource: " << (*it).name()
                     << "[" << (*it).uri << "]"
                     << " (->" << (*it).parent->name() << ")"
                     << ( (*it).generateClass() ? " (will be generated)" : " (will not be generated)" )
                     << endl;

        Q_ASSERT( d->resources.count( it.key() ) == 1 );

        QListIterator<const Property*> propIt( (*it).properties );
        while( propIt.hasNext() ) {
            const Property* p = propIt.next();
            if ( !quiet )
                qDebug() << "          " << p->uri << " (->" << p->type << ")" << ( p->list ? QString("+") : QString("1") ) << endl;
        }
    }

    return success;
}


bool OntologyParser::writeSources( const QString& dir, bool externalRefs )
{
    bool success = true;

    for( QMap<QString, ResourceClass>::const_iterator it = d->resources.constBegin();
         it != d->resources.constEnd(); ++it ) {
        if( (*it).generateClass() )
            success &= (*it).write( dir + QDir::separator() );
    }

    return success;
}


QStringList OntologyParser::listHeader()
{
    QStringList l;
    for( QMap<QString, ResourceClass>::const_iterator it = d->resources.constBegin();
         it != d->resources.constEnd(); ++it )
        if( (*it).generateClass() )
            l.append( (*it).headerName() );
    return l;
}


QStringList OntologyParser::listSources()
{
    QStringList l;
    for( QMap<QString, ResourceClass>::const_iterator it = d->resources.constBegin();
         it != d->resources.constEnd(); ++it )
        if( (*it).generateClass() )
            l.append( (*it).sourceName() );
    return l;
}

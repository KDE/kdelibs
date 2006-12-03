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

#include "ontologyparser.h"
#include "resourceclass.h"

#include <soprano/Soprano.h>

#include <QtCore>


class OntologyParser::Private
{
public:
  QMap<QString, ResourceClass> resources;
  QMap<QString, Property> properties;
  QMap<QString, QString> comments;
  Soprano::Parser* rdfParser;

  ResourceClass& getResource( const QString& uri ) {
    ResourceClass& r = resources[uri];
    r.uri = uri;
    return r;
  }

  Property& getProperty( const QString& uri ) {
    Property& p = properties[uri];
    p.uri = uri;
    return p;
  }
};


OntologyParser::OntologyParser()
{
  d = new Private;
  d->rdfParser = Soprano::Manager::instance()->factory( "Redland" )->createParser();
}


OntologyParser::~OntologyParser()
{
  delete d->rdfParser;
  delete Soprano::Manager::instance();
  delete d;
}


bool OntologyParser::parse( const QString& filename )
{
  qDebug() << "(OntologyParser) Parsing " << filename << endl;

  Soprano::Model* model = d->rdfParser->parse( filename );
  bool success = true;

  if( !model )
    return false;

  Soprano::StatementIterator* it = model->listStatements();
  while( it->hasNext() ) {
    const Soprano::Statement& s = it->next();

    if( s.predicate().uri().toString().endsWith( "#subClassOf" ) ) {
      ResourceClass& rc = d->getResource( s.subject().uri().toString() );
      rc.parent = &d->getResource( s.object().uri().toString() );
    }
    else if( s.predicate().uri().toString().endsWith( "#type" ) ) {
      if( s.object().uri().toString().endsWith( "#Class" ) )
	d->getResource( s.subject().uri().toString() );
    }
    else if( s.predicate().uri().toString().endsWith( "#domain" ) ) {
      ResourceClass& rc = d->getResource( s.object().uri().toString() );
      Property& p = d->getProperty( s.subject().uri().toString() );
      rc.properties[s.subject().uri().toString()] = &p;
    }
    else if( s.predicate().uri().toString().endsWith( "#range" ) ) {
      d->properties[s.subject().uri().toString()].type = s.object().uri().toString();
    }
    else if( s.predicate().uri().toString().endsWith( "#maxCardinality" ) ) {
      d->properties[s.subject().uri().toString()].list = ( s.object().literal().toInt() > 1 );
    }
    else if( s.predicate().uri().toString().endsWith( "#comment" ) ) {
      d->comments[s.subject().uri().toString()] = s.object().literal();
    }
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

  delete it;
  delete model;

  // testing stuff
//   for( QMap<QString, ResourceClass>::const_iterator it = d->resources.constBegin();
//        it != d->resources.constEnd(); ++it ) {
//     qDebug() << "Resource: " << (*it).name() << " (->" << (*it).parent->name() << ")" << endl;

//     QMapIterator<QString, Property*> propIt( (*it).properties );
//     while( propIt.hasNext() ) {
//       propIt.next();
//       const Property* p = propIt.value();
//       qDebug() << "          " << p->uri << " (->" << p->type << ")" << ( p->list ? QString("+") : QString("1") ) << endl;
//     }
//   }

  return success;
}


bool OntologyParser::writeSources( const QString& dir )
{
  bool success = true;

  for( QMap<QString, ResourceClass>::const_iterator it = d->resources.constBegin();
       it != d->resources.constEnd(); ++it ) {
    success &= (*it).write( dir );
  }

  return success;
}


QStringList OntologyParser::listHeader()
{
  QStringList l;
  for( QMap<QString, ResourceClass>::const_iterator it = d->resources.constBegin();
       it != d->resources.constEnd(); ++it )
    l.append( (*it).headerName() );
  return l;
}


QStringList OntologyParser::listSources()
{
  QStringList l;
  for( QMap<QString, ResourceClass>::const_iterator it = d->resources.constBegin();
       it != d->resources.constEnd(); ++it )
    l.append( (*it).sourceName() );
  return l;
}

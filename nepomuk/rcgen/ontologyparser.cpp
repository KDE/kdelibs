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
#include "resourcetemplate.h"

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
  d->rdfParser = Soprano::createParser();
  Q_ASSERT( d->rdfParser );
}


OntologyParser::~OntologyParser()
{
  delete d->rdfParser;
  delete d;
}


bool OntologyParser::parse( const QString& filename )
{
  qDebug() << "(OntologyParser) Parsing " << filename << endl;

  Soprano::Model* model = d->rdfParser->parse( filename );
  bool success = true;

  if( !model )
    return false;

  Soprano::StatementIterator it = model->listStatements();
  while( it.hasNext() ) {
    const Soprano::Statement& s = it.next();

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
      p.domain = &rc;
      rc.properties.append( &p );
    }
    else if( s.predicate().uri().toString().endsWith( "#range" ) ) {
      d->properties[s.subject().uri().toString()].type = s.object().uri().toString();
    }
    else if( s.predicate().uri().toString().endsWith( "#maxCardinality" ) ||
	     s.predicate().uri().toString().endsWith( "#cardinality" ) ) {
      d->properties[s.subject().uri().toString()].list = ( s.object().literal().toInt() > 1 );
    }
    else if( s.predicate().uri().toString().endsWith( "#comment" ) ) {
      d->comments[s.subject().uri().toString()] = s.object().literal();
    }
  }

  delete model;

  // determine the reverse properties
  for( QMap<QString, Property>::const_iterator propIt = d->properties.constBegin();
       propIt != d->properties.constEnd(); ++propIt ) {
    const Property& p = propIt.value();
    if( d->resources.contains( p.type ) ) {
      qDebug() << "Setting reverse property " << p.uri << " on type " << p.type << endl;
      d->resources[p.type].reverseProperties.append( &p );
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
    if( (*it).name() != "Resource" )
      success &= (*it).write( dir + '/' );
  }

  return success;
}


QStringList OntologyParser::listHeader()
{
  QStringList l;
  for( QMap<QString, ResourceClass>::const_iterator it = d->resources.constBegin();
       it != d->resources.constEnd(); ++it )
    if( (*it).name() != "Resource" )
      l.append( (*it).headerName() );
  return l;
}


QStringList OntologyParser::listSources()
{
  QStringList l;
  for( QMap<QString, ResourceClass>::const_iterator it = d->resources.constBegin();
       it != d->resources.constEnd(); ++it )
    if( (*it).name() != "Resource" )
      l.append( (*it).sourceName() );
  return l;
}


bool OntologyParser::writeOntology( const QString& dir )
{
  QString s;

  // types and their properties
  for( QMap<QString, ResourceClass>::const_iterator it = d->resources.constBegin();
       it != d->resources.constEnd(); ++it ) {
    s.append( "   d->types.append( \"" + it.value().uri + "\" );\n" );
    for( QList<const Property*>::const_iterator it2 = it.value().properties.constBegin();
	 it2 != it.value().properties.constEnd(); ++it2 ) {
      s.append( "   d->properties[\"" + it.value().uri + "\"].append( \"" + (*it2)->uri + "\" );\n" );
    }
  }

  // Resource inheritance
  for( QMap<QString, ResourceClass>::const_iterator it = d->resources.constBegin();
       it != d->resources.constEnd(); ++it ) {
    s.append( "   d->inheritanceGraph[ \"" + it.value().uri + "\" ] = \"" + it.value().parent->uri +"\";\n" );
  }

  // FIXME: type inheritance

  QString ctor = ontologySrcTemplate;
  ctor.replace( "CONSTRUCTOR", s );

  QFile f( dir + "/ontology_ctor.cpp" );
  if( f.open( QIODevice::WriteOnly ) ) {
    QTextStream fs( &f );
    fs << ctor;
    return true;
  }

  return false;
}

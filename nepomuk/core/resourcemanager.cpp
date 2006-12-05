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

#include "resourcemanager.h"
#include "resource.h"
#include "resourcedata.h"
#include "ontology.h"

#include <knep/knep.h>
#include <knep/services/tripleservice.h>
#include <knep/services/statementlistiterator.h>

#include <kstaticdeleter.h>

using namespace Nepomuk::Backbone::Services;
using namespace Nepomuk::Backbone::Services::RDF;


class Nepomuk::KMetaData::ResourceManager::Private
{
public:
  Private()
    : autoSync(true) {
  }

  bool autoSync;
  Nepomuk::Backbone::Registry* registry;
};


Nepomuk::KMetaData::ResourceManager::ResourceManager()
  : QObject()
{
  d = new Private();
  d->registry = new Backbone::Registry( this );
}


Nepomuk::KMetaData::ResourceManager::~ResourceManager()
{
  delete d;
}


KStaticDeleter<Nepomuk::KMetaData::ResourceManager> s_resourceManagerDeleter;

Nepomuk::KMetaData::ResourceManager* Nepomuk::KMetaData::ResourceManager::instance()
{
  static ResourceManager* s_instance = 0;
  if( !s_instance )
    s_resourceManagerDeleter.setObject( s_instance, new ResourceManager() );
  return s_instance;
}


int Nepomuk::KMetaData::ResourceManager::init()
{
//   if( serviceRegistry()->status() != VALID ) {
//     qDebug() << "(ResourceManager) failed to initialize registry." << endl;
//     return -1;
//  }

  if( !serviceRegistry()->discoverTripleService() ) {
    qDebug() << "(ResourceManager) No NEPOMUK Triple service found." << endl;
    return -1;
  }

  if( !serviceRegistry()->discoverResourceIdService() ) {
    qDebug() << "(ResourceManager) No NEPOMUK ResourceId service found." << endl;
    return -1;
  }

  return 0;
}


Nepomuk::Backbone::Registry* Nepomuk::KMetaData::ResourceManager::serviceRegistry() const
{
  return d->registry;
}


bool Nepomuk::KMetaData::ResourceManager::autoSync() const
{
  return d->autoSync;
}


Nepomuk::KMetaData::Resource Nepomuk::KMetaData::ResourceManager::createResourceFromUri( const QString& uri )
{
  return Resource( uri, QString() );
}


void Nepomuk::KMetaData::ResourceManager::setAutoSync( bool enabled )
{
  d->autoSync = enabled;
}


void Nepomuk::KMetaData::ResourceManager::syncAll()
{
  for( QHash<QString, ResourceData*>::iterator it = ResourceData::s_data.begin();
       it != ResourceData::s_data.end(); ++it ) {
    if( it.value()->merge() )
      it.value()->save();
  }
}


QList<Nepomuk::KMetaData::Resource> Nepomuk::KMetaData::ResourceManager::allResourcesOfType( const QString& type ) const
{
  QList<Resource> l;

  TripleService ts( serviceRegistry()->discoverTripleService() );
  StatementListIterator it( ts.listStatements( Ontology::defaultGraph(), 
					       Statement( Node(), Ontology::typePredicate(), Node(type) ) ), 
			    &ts );
  while( it.hasNext() )
    l.append( Resource( it.next().subject.value ) );

  return l;
}

#include "resourcemanager.moc"

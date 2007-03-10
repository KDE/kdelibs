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
#include "resourcedata.h"
#include "ontology.h"
#include "tools.h"

#include <kmetadata/resource.h>

#include <knep/knep.h>
#include <knep/services/rdfrepository.h>
#include <knep/rdf/statementlistiterator.h>

#include <kstaticdeleter.h>
#include <kdebug.h>

#include <qthread.h>


using namespace Nepomuk::Services;
using namespace Nepomuk::RDF;


class Nepomuk::KMetaData::ResourceManager::Private : public QThread
{
public:
  Private( ResourceManager* manager )
    : autoSync(true),
      m_parent(manager) {
  }

  void run() {
    m_parent->syncAll();
  }

  bool autoSync;
  Nepomuk::Backbone::Registry* registry;
  Nepomuk::KMetaData::Ontology* ontology;

  QTimer syncTimer;

private:
  ResourceManager* m_parent;
};


Nepomuk::KMetaData::ResourceManager::ResourceManager()
  : QObject()
{
  d = new Private( this );
  d->ontology = new Ontology();
  d->registry = new Backbone::Registry( this );

  connect( &d->syncTimer, SIGNAL(timeout()),
	   this, SLOT(slotStartAutoSync()) );

  setAutoSync( true );
}


Nepomuk::KMetaData::ResourceManager::~ResourceManager()
{
  // do one last sync
  d->syncTimer.stop();
  triggerSync();
  d->wait();
  delete d->ontology;
  delete d;
}


KStaticDeleter<Nepomuk::KMetaData::ResourceManager> s_resourceManagerDeleter;

// FIXME: make the singleton deletion thread-safe so autosyncing will be forced when shutting
//        down the application
Nepomuk::KMetaData::ResourceManager* Nepomuk::KMetaData::ResourceManager::instance()
{
  static ResourceManager* s_instance = 0;
  if( !s_instance )
    s_instance = new ResourceManager();
  //  s_resourceManagerDeleter.setObject( s_instance, new ResourceManager() );
  return s_instance;
}


int Nepomuk::KMetaData::ResourceManager::init()
{
//   if( serviceRegistry()->status() != VALID ) {
//     kDebug(300004) << "(ResourceManager) failed to initialize registry." << endl;
//     return -1;
//  }

  if( !serviceRegistry()->discoverRDFRepository() ) {
    kDebug(300004) << "(ResourceManager) No NEPOMUK RDFRepository service found." << endl;
    return -1;
  }

  if( !serviceRegistry()->discoverResourceIdService() ) {
    kDebug(300004) << "(ResourceManager) No NEPOMUK ResourceId service found." << endl;
    return -1;
  }

  return 0;
}


Nepomuk::KMetaData::Ontology* Nepomuk::KMetaData::ResourceManager::ontology() const
{
  return d->ontology;
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


void Nepomuk::KMetaData::ResourceManager::slotStartAutoSync()
{
  if( d->autoSync )
    triggerSync();
}


void Nepomuk::KMetaData::ResourceManager::setAutoSync( bool enabled )
{
  d->autoSync = enabled;
}


void Nepomuk::KMetaData::ResourceManager::syncAll()
{
  kDebug(300004) << k_funcinfo << endl;

  //
  // Gather all information to be synced and add it in one go
  //
  // FIXME: use some upper bound, i.e. never add more than N statements at once

  RDFRepository rr( serviceRegistry()->discoverRDFRepository() );

  // make sure our graph exists
  // ==========================
  if( !rr.listRepositoriyIds().contains( KMetaData::defaultGraph() ) ) {
    rr.createRepository( KMetaData::defaultGraph() );
  }

  // Get URIs for everyone
  // =====================
  for( QHash<QString, ResourceData*>::iterator it = ResourceData::s_kickoffData.begin();
       it != ResourceData::s_kickoffData.end(); ++it )
    it.value()->init();

  QList<Statement> statementsToAdd;
  QList<ResourceData*> syncedResources;
  for( QHash<QString, ResourceData*>::iterator it = ResourceData::s_data.begin();
       it != ResourceData::s_data.end(); ++it ) {
    syncedResources.append( it.value() );
    it.value()->startSync();
    it.value()->determinePropertyUris();
    statementsToAdd += it.value()->allStatementsToAdd();
  }

  // remove everything about everyone from the store
  // this is the stupid way but ATM the only working one
  // ===================================================
  for( QHash<QString, ResourceData*>::iterator it = ResourceData::s_data.begin();
       it != ResourceData::s_data.end(); ++it ) {
    rr.removeAllStatements( KMetaData::defaultGraph(), Statement( it.value()->uri(), Node(), Node() ) );
    if( !rr.success() )
      kDebug(300004) << "(ResourceManager) failed to remove all statements for resource " << it.value()->uri()
		     << " (" << rr.lastErrorName() << ")" << endl;
  }

  rr.addStatements( KMetaData::defaultGraph(), statementsToAdd );

  if( !rr.success() )
    kDebug(300004) << "(ResourceManager) failed to add statements"
		   << " (" << rr.lastErrorName() << ")" << endl;

  bool success = rr.success();

  //
  // Release all the resource datas.
  //
  for( QList<ResourceData*>::iterator it = syncedResources.begin();
       it != syncedResources.end(); ++it ) {
    ResourceData* data = *it;
    data->endSync( success );
    if( !data->modified() && !data->cnt() )
      data->deleteData();
  }
}


void Nepomuk::KMetaData::ResourceManager::triggerSync()
{
  if( !d->isRunning() )
    d->start();
}


void Nepomuk::KMetaData::ResourceManager::notifyError( const QString& uri, int errorCode )
{
  kDebug(300004) << "(Nepomuk::KMetaData::ResourceManager) error: " << uri << " " << errorCode << endl;
  emit error( uri, errorCode );
}


QList<Nepomuk::KMetaData::Resource> Nepomuk::KMetaData::ResourceManager::allResourcesOfType( const QString& type ) const
{
  QList<Resource> l;

  if( !type.isEmpty() ) {
    // check local data
    for( QHash<QString, ResourceData*>::iterator rdIt = ResourceData::s_kickoffData.begin();
	 rdIt != ResourceData::s_kickoffData.end(); ++rdIt ) {

      if( rdIt.value()->type() == type ) {
	l.append( Resource( rdIt.key() ) );
      }
    }

    kDebug(300004) << k_funcinfo << " added local resources: " << l.count() << endl;

    // check remote data
    RDFRepository rdfr( serviceRegistry()->discoverRDFRepository() );
    StatementListIterator it( rdfr.queryListStatements( KMetaData::defaultGraph(), 
							Statement( Node(), KMetaData::typePredicate(), Node(type) ),
							100 ), 
			      &rdfr );
    while( it.hasNext() ) {
      const Statement& s = it.next();
      Resource res( s.subject.value );
      if( !l.contains( res ) )
	l.append( res );
    }

    kDebug(300004) << k_funcinfo << " added remote resources: " << l.count() << endl;
  }

  return l;
}


QList<Nepomuk::KMetaData::Resource> Nepomuk::KMetaData::ResourceManager::allResourcesWithProperty( const QString& _uri, const Variant& v ) const
{
  QString uri = ensureNamespace( _uri );

  QList<Resource> l;

  if( v.isList() ) {
    kDebug(300004) << "(ResourceManager::allResourcesWithProperty) list values not supported." << endl;
  }
  else {
    // check local data
    for( QHash<QString, ResourceData*>::iterator rdIt = ResourceData::s_kickoffData.begin();
	 rdIt != ResourceData::s_kickoffData.end(); ++rdIt ) {

      if( rdIt.value()->hasProperty( uri ) &&
	  rdIt.value()->getProperty( uri ) == v )
	l.append( Resource( rdIt.key() ) );
    }

    // check remote data
    RDFRepository rdfr( serviceRegistry()->discoverRDFRepository() );
    Node n;
    if( v.isResource() ) {
      n.value = v.toResource().uri();
      n.type = RDF::NodeResource;
    }
    else {
      n = KMetaData::valueToRDFNode( v );
    }
    
    StatementListIterator it( rdfr.queryListStatements( KMetaData::defaultGraph(), 
							Statement( Node(), Node(uri), n ),
							100 ), &rdfr );
    
    while( it.hasNext() ) {
      const Statement& s = it.next();
      Resource res( s.subject.value );
      if( !l.contains( res ) )
	l.append( res );
    }
  }

  return l;    
}


QString Nepomuk::KMetaData::ResourceManager::generateUniqueUri() const
{
  RDFRepository rdfr( serviceRegistry()->discoverRDFRepository() );

  QString s;
  while( 1 ) {
    s = ontology()->defaultNamespace() + '#' + KRandom::randomString( 20 );
    if( !rdfr.listRepositoriyIds().contains( KMetaData::defaultGraph() ) ||
	( !rdfr.contains( KMetaData::defaultGraph(), Statement( s, Node(), Node() ) ) &&
	  !rdfr.contains( KMetaData::defaultGraph(), Statement( Node(), s, Node() ) ) &&
	  !rdfr.contains( KMetaData::defaultGraph(), Statement( Node(), Node(), s ) ) ) )
      return s;
  }
}

#include "resourcemanager.moc"

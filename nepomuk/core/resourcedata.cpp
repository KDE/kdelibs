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

#include "resourcedata.h"
#include "resourcemanager.h"
#include "tools.h"
#include "ontology.h"

#include <knep/knep.h>
#include <knep/services/rdfrepository.h>
#include <knep/services/resourceidservice.h>
#include <knep/rdf/statementlistiterator.h>

#include <kdebug.h>

#include <QMutex>


using namespace Nepomuk::Services;
using namespace Nepomuk::RDF;

QHash<QString, Nepomuk::KMetaData::ResourceData*> Nepomuk::KMetaData::ResourceData::s_data;


Nepomuk::KMetaData::ResourceData::ResourceData( const QString& uri_, const QString& type_ )
  : m_uri( uri_ ),
    m_type( type_ ),
    m_flags(0),
    m_ref(0),
    m_initialized( false )
{
  if( !m_uri.isEmpty() && m_type.isEmpty() )
    m_type = ResourceManager::instance()->ontology()->defaultType();
}


Nepomuk::KMetaData::ResourceData::~ResourceData()
{
}


const QString& Nepomuk::KMetaData::ResourceData::uri() const
{
  return m_uri;
}


const QString& Nepomuk::KMetaData::ResourceData::type() const
{
  return m_type;
}


void Nepomuk::KMetaData::ResourceData::deleteData()
{
  s_data.remove( m_uri );
  delete this;
}


bool Nepomuk::KMetaData::ResourceData::init()
{
  if( !m_initialized )
    m_initialized = merge();//load();
  return m_initialized;
}


QHash<QString, Nepomuk::KMetaData::Variant> Nepomuk::KMetaData::ResourceData::allProperties() const
{
  QHash<QString, Variant> l;
  for( PropertiesMap::const_iterator it = m_properties.constBegin();
       it != m_properties.constEnd(); ++it )
    l.insert( it.key(), it.value().first );
  return l;
}


bool Nepomuk::KMetaData::ResourceData::hasProperty( const QString& uri ) const
{
  return m_properties.contains( uri );
}


Nepomuk::KMetaData::Variant Nepomuk::KMetaData::ResourceData::getProperty( const QString& uri ) const
{
  PropertiesMap::const_iterator it = m_properties.constFind( uri );
  if( it != m_properties.end() )
    if( !( it.value().second & ResourceData::Removed ) )
      return it.value().first;
  
  return Variant();
}


void Nepomuk::KMetaData::ResourceData::setProperty( const QString& uri, const Nepomuk::KMetaData::Variant& value )
{
  m_modificationMutex.lock();

  // reset the deleted flag
  m_flags = 0;

  // mark the value as modified
  m_properties[uri] = qMakePair<Variant, int>( value, ResourceData::Modified );

  m_modificationMutex.unlock();
}


void Nepomuk::KMetaData::ResourceData::removeProperty( const QString& uri )
{
  m_modificationMutex.lock();

  ResourceData::PropertiesMap::iterator it = m_properties.find( uri );
  if( it != m_properties.end() )
    it.value().second = ResourceData::Modified|ResourceData::Removed;

  m_modificationMutex.unlock();
}


void Nepomuk::KMetaData::ResourceData::remove()
{
  m_modificationMutex.lock();

  m_flags |= Removed;

  m_modificationMutex.unlock();
}


void Nepomuk::KMetaData::ResourceData::revive()
{
  m_modificationMutex.lock();

  m_flags = 0;

  m_modificationMutex.unlock();
}


bool Nepomuk::KMetaData::ResourceData::modified() const
{
  // If the resource is Removed it has not been synced yet and thus is modified
  // If it is only marked as Deleted it has been synced and thus is not modified
  if( m_flags & Removed )
    return true;

  for( ResourceData::PropertiesMap::const_iterator it = m_properties.constBegin();
       it != m_properties.constEnd(); ++it )
    if( it.value().second & ResourceData::Modified )
      return true;

  return false;
}


bool Nepomuk::KMetaData::ResourceData::removed() const
{
  return (m_flags & Removed|Deleted);
}


bool Nepomuk::KMetaData::ResourceData::exists() const
{
  if( isValid() ) {
    RDFRepository rr( ResourceManager::instance()->serviceRegistry()->discoverRDFRepository() );
    
    // the resource has to exists either as a subject or as an object
    return( rr.contains( KMetaData::defaultGraph(), Statement( m_uri, Node(), Node() ) ) || 
	    rr.contains( KMetaData::defaultGraph(), Statement( Node(), Node(), m_uri ) ) );
  }
  else
    return false;
}


bool Nepomuk::KMetaData::ResourceData::isValid() const
{
  // FIXME: check namespaces and stuff
  return( !m_uri.isEmpty() && !m_type.isEmpty() );
}


bool Nepomuk::KMetaData::ResourceData::inSync()
{
  init();

  ResourceData* currentData = new ResourceData( m_uri );
  bool ins = ( currentData->load() && *currentData == *this );
  delete currentData;
  return ins;
}


bool Nepomuk::KMetaData::ResourceData::load()
{
  if( isValid() ) {
    m_modificationMutex.lock();

    m_properties.clear();
    m_flags = 0;

    RDFRepository rr( ResourceManager::instance()->serviceRegistry()->discoverRDFRepository() );

    StatementListIterator it( rr.queryListStatements( KMetaData::defaultGraph(), 
						      Statement( m_uri, Node(), Node() ),
						      0 ), 
			      &rr );
    while( it.hasNext() ) {
      const Statement& s = it.next();

      // load the type if we have no type or the default
      if( s.predicate.value == KMetaData::typePredicate() ) {
	if( m_type.isEmpty() || m_type == ResourceManager::instance()->ontology()->defaultType() )
	  m_type = s.object.value;
      }
      else {
	PropertiesMap::iterator oldProp = m_properties.find( s.predicate.value );
	if( s.object.type == NodeResource ) {
	  if( !loadProperty( s.predicate.value, Resource( s.object.value ) ) ) {
	    m_modificationMutex.unlock();
	    return false;
	  }
	}
	else {
	  if( !loadProperty( s.predicate.value, KMetaData::RDFLiteralToValue( s.object.value ) ) ) {
	    m_modificationMutex.unlock();
	    return false;
	  }
	}
      }
    }

    m_modificationMutex.unlock();
   
    return true;
  }
  else
    return false;
}


bool Nepomuk::KMetaData::ResourceData::loadProperty( const QString& name, const Variant& val )
{
  PropertiesMap::iterator oldProp = m_properties.find( name );
  if( oldProp == m_properties.end() ) {
    m_properties.insert( name, qMakePair<Variant, int>( val, 0 ) );
  }
  else if( val.type() != oldProp.value().first.simpleType() ) {
    ResourceManager::instance()->notifyError( m_uri, ERROR_INVALID_TYPE );
    return false;
  }
  else {
    oldProp.value().first.append( val );
  }

  return true;
}


void Nepomuk::KMetaData::ResourceData::startSync()
{
  m_syncingMutex.lock();
  m_modificationMutex.lock();

  //
  // Set all properties in the syncing state.
  // The properties that will be changed during the sync will have
  // their syncing flag removed and will thus stay marked modified
  //
  for( PropertiesMap::iterator it = m_properties.begin();
       it != m_properties.end(); ++it ) {
    it.value().second &= Syncing;
  }

  m_modificationMutex.unlock();
}


void Nepomuk::KMetaData::ResourceData::endSync( bool updateFlags )
{
  m_modificationMutex.lock();

  //
  // Remove all syncing flags.
  // Those properties that are still marked as Syncing were
  // not modified during the sync and can thus be marked as so.
  //
  for( PropertiesMap::iterator it = m_properties.begin();
       it != m_properties.end(); ++it ) {
    if( it.value().second & Syncing ) {
      it.value().second &= ~Syncing;
      // FIXME: should we actually delete properties marked as Removed here?
      if( updateFlags )
	it.value().second &= ~Modified;
    }
  }

  m_modificationMutex.unlock();

  m_syncingMutex.unlock();
}


bool Nepomuk::KMetaData::ResourceData::save()
{
  RDFRepository rr( ResourceManager::instance()->serviceRegistry()->discoverRDFRepository() );

  m_modificationMutex.lock();

  // make sure our graph exists
  // ==========================
  if( !rr.listRepositoriyIds().contains( KMetaData::defaultGraph() ) ) {
    rr.createRepository( KMetaData::defaultGraph() );
//     if( rr.createRepository( KMetaData::defaultGraph() ) ) {
//       ResourceManager::instance()->notifyError( m_uri, ERROR_COMMUNICATION );
//       m_modificationMutex.unlock();
//       return false;
//     }
  }

  // if the resource has been deleted locally we are done after removing it
  // ======================================================================
  if( m_flags & (Removed|Deleted) ) {

    // remove everything about this resource from the store
    // ====================================================
    rr.removeAllStatements( KMetaData::defaultGraph(), Statement( m_uri, Node(), Node() ) );
//     if( rr.removeAllStatements( KMetaData::defaultGraph(), Statement( m_uri, Node(), Node() ) ) ) {
//       kDebug(300004) << "(ResourceData) removing all statements of resource " << m_uri << " failed." << endl;
//       ResourceManager::instance()->notifyError( m_uri, ERROR_COMMUNICATION );
//       m_modificationMutex.unlock();
//       return false;
//     }

    // FIXME: allow revive even after a sync
    m_properties.clear();
    m_flags = Deleted;
  }
  else {

    // remove all the deleted properties from the store
    // ================================================
    QList<Statement> sl = allStatementsToRemove();
    if( !sl.isEmpty() ) {
      rr.removeStatements( KMetaData::defaultGraph(), sl );
//       if( rr.removeStatements( KMetaData::defaultGraph(), sl ) ) {
// 	kDebug(300004) << "(ResourceData) removing statements for resource " << m_uri << " failed." << endl;
// 	ResourceManager::instance()->notifyError( m_uri, ERROR_COMMUNICATION );
// 	m_modificationMutex.unlock();
// 	return false;
//       }
    }

    // save all statements into the store
    // ==================================
    sl = allStatementsToAdd();
    if( !sl.isEmpty() ) {
      rr.addStatements( KMetaData::defaultGraph(), sl );
//       if( rr.addStatements( KMetaData::defaultGraph(), sl ) ) {
// 	kDebug(300004) << "(ResourceData) adding statements for resource " << m_uri << " failed." << endl;
// 	ResourceManager::instance()->notifyError( m_uri, ERROR_COMMUNICATION );
// 	m_modificationMutex.unlock();
// 	return false;
//       }
    }

    for( PropertiesMap::iterator it = m_properties.begin();
	 it != m_properties.end(); ++it ) {
      // whatever gets saved is not Modified anymore
      it.value().second &= ~Modified;
    }
  }

  m_modificationMutex.unlock();

  return true;
}


QList<Nepomuk::RDF::Statement> Nepomuk::KMetaData::ResourceData::allStatements( int flags ) const
{
  QList<Statement> statements;

  // save the properties
  // ===================
  for( PropertiesMap::const_iterator it = m_properties.constBegin();
       it != m_properties.constEnd(); ++it ) {

    if( it.value().second & flags ) {

      QString predicate = it.key();
      const Variant& val = it.value().first;

      // one-to-one Resource
      if( val.isResource() ) {
	statements.append( Statement( m_uri, predicate, val.toResource().uri() ) );
      }

      // one-to-many Resource
      else if( val.isResourceList() ) {
	const QList<Resource>& l = val.toResourceList();
	for( QList<Resource>::const_iterator resIt = l.constBegin(); resIt != l.constEnd(); ++resIt ) {
	  statements.append( Statement( m_uri, predicate, (*resIt).uri() ) );
	}
      }

      // one-to-many literals
      else if( val.isList() ) {
	QStringList l = KMetaData::valuesToRDFLiterals( val );
	for( QStringList::const_iterator valIt = l.constBegin(); valIt != l.constEnd(); ++valIt ) {
	  statements.append( Statement( m_uri, predicate, Node( *valIt, NodeLiteral ) ) );
	}
      }

      // one-to-one literal
      else {
	statements.append( Statement( m_uri, 
				      predicate, 
				      Node( KMetaData::valueToRDFLiteral( val ), NodeLiteral ) ) );
      }
    }
  }

  return statements;
}


QList<Nepomuk::RDF::Statement> Nepomuk::KMetaData::ResourceData::allStatementsToAdd() const
{
  QList<Statement> statements = allStatements( Modified );

  // always save the type
  // ====================
  statements.append( Statement( Node(m_uri), Node(KMetaData::typePredicate()), Node(m_type) ) );

  return statements;
}


QList<Nepomuk::RDF::Statement> Nepomuk::KMetaData::ResourceData::allStatementsToRemove() const
{
  return allStatements( Removed|Deleted );
}


/**
 * Merge in changes from the local store
 */
bool Nepomuk::KMetaData::ResourceData::merge()
{
  // TODO: do more intelligent syncing
  ResourceData* currentData = new ResourceData( m_uri );
    
  if( currentData->load() ) {
    m_modificationMutex.lock();

    // merge in possible remote changes
    for( PropertiesMap::const_iterator it = currentData->m_properties.constBegin();
	 it != currentData->m_properties.constEnd(); ++it ) {

      // 1. the value exists here and has not been changed

      // 1.1 the value is the same
      //     -> do nothing

      // 1.2 the value differs
      //     -> load the new value from currentData

      // 2. the value exists here and has been changed (Problem: since we did not remember the
      //    original value we don't know if it was changed remotely in the meantime. Is that 
      //    really a problem? If so could we use timestamps?)
      //    -> keep our value

      // 3. the value does not exist here
      //    -> copy it over

      PropertiesMap::iterator it2 = this->m_properties.find( it.key() );

      if( it2 != this->m_properties.constEnd() ) {
	if( !(it2.value().second & Modified) ) {
	  it2.value().first = it.value().first;
	}
      }
      else {
	this->m_properties.insert( it.key(), it.value() );
      }
    }

    PropertiesMap::iterator it = this->m_properties.begin();
    while( it != this->m_properties.end() ) {

      // 4. a value that exists here does not exist there (second for-loop?)

      // 4.1 it has been modified here
      //     -> keep it

      // 4.2 it has not been modified here
      //     -> remove it since it has been deleted in the meantime

      PropertiesMap::const_iterator it2 = currentData->m_properties.constFind( it.key() );
      if( it2 == currentData->m_properties.constEnd() &&
	  !(it.value().second & Modified) ) {
	it = this->m_properties.erase( it );
      }
      else {
	++it;
      }
    }

    delete currentData;

    m_modificationMutex.unlock();

    return true;
  }
    
  delete currentData;
  return false;
}


/**
 * Compares the properties of two ResourceData objects taking into account the Removed flag
 */
bool Nepomuk::KMetaData::ResourceData::operator==( const ResourceData& other ) const
{
  if( this == &other )
    return true;

  if( this->m_uri != other.m_uri ||
      this->m_type != other.m_type )
    return false;

  // Evil is among us!
  const_cast<ResourceData*>(this)->init();
  const_cast<ResourceData*>(&other)->init();

  if( exists() && m_flags != other.m_flags )
    return false;

  for( PropertiesMap::const_iterator it = other.m_properties.constBegin();
       it != other.m_properties.constEnd(); ++it ) {
    PropertiesMap::const_iterator it2 = this->m_properties.constFind( it.key() );

    // 1. the property does not exist here
    if( it2 == this->m_properties.constEnd() )
      return false;
    // 2. the values differ or it has been removed here
    if( it.value().first != it2.value().first ||
	it2.value().second & Removed )
      return false;
  }

  for( PropertiesMap::const_iterator it = this->m_properties.constBegin();
       it != this->m_properties.constEnd(); ++it ) {
    // 3. the property does not exist there
    if( other.m_properties.constFind( it.key() ) == this->m_properties.constEnd() )
      return false;
    // 4. the values differ (already handled above)
  }

  return true;
}


Nepomuk::KMetaData::ResourceData* Nepomuk::KMetaData::ResourceData::data( const QString& uri, const QString& type )
{
  Services::ResourceIdService resids( ResourceManager::instance()->serviceRegistry()->discoverResourceIdService() );
  QString uniqueUri = resids.toUniqueUrl( uri );
  if( uniqueUri.isEmpty() ) {
    kDebug(300004) << "(ResourceData) determining unique URI failed. Falling back to plain URI." << endl;
    uniqueUri = uri;
  }

  QHash<QString, ResourceData*>::iterator it = s_data.find( uniqueUri );
  if( it == s_data.end() ) {
    ResourceData* d = new ResourceData( uniqueUri, type );

    s_data.insert( uniqueUri, d );

    return d;
  }
  else {
    return it.value();
  }
}



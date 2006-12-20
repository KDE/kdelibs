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
#include <knep/services/tripleservice.h>
#include <knep/services/resourceidservice.h>
#include <knep/services/statementlistiterator.h>


using namespace Nepomuk::Backbone::Services;
using namespace Nepomuk::Backbone::Services::RDF;

QHash<QString, Nepomuk::KMetaData::ResourceData*> Nepomuk::KMetaData::ResourceData::s_data;


Nepomuk::KMetaData::ResourceData::ResourceData( const QString& uri_, const QString& type_ )
  : m_uri( uri_ ),
    m_type( type_ ),
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
    m_initialized = load();
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
    if( !( it.value().second & ResourceData::Deleted ) )
      return it.value().first;
  
  return Variant();
}


void Nepomuk::KMetaData::ResourceData::setProperty( const QString& uri, const Nepomuk::KMetaData::Variant& value )
{
  // mark the value as modified
  m_properties[uri] = qMakePair<Variant, int>( value, ResourceData::Modified );
}


void Nepomuk::KMetaData::ResourceData::removeProperty( const QString& uri )
{
  ResourceData::PropertiesMap::iterator it = m_properties.find( uri );
  if( it != m_properties.end() )
    it.value().second = ResourceData::Modified|ResourceData::Deleted;
}


bool Nepomuk::KMetaData::ResourceData::modified() const
{
  for( ResourceData::PropertiesMap::const_iterator it = m_properties.constBegin();
       it != m_properties.constEnd(); ++it )
    if( it.value().second & ResourceData::Modified )
      return true;

  return false;
}


bool Nepomuk::KMetaData::ResourceData::exists() const
{
  if( isValid() ) {
    TripleService ts( ResourceManager::instance()->serviceRegistry()->discoverTripleService() );
    
    // the resource has to exists either as a subject or as an object
    return( !ts.contains( KMetaData::defaultGraph(), Statement( m_uri, Node(), Node() ) ) || 
	    !ts.contains( KMetaData::defaultGraph(), Statement( Node(), Node(), m_uri ) ) );
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
    m_properties.clear();

    TripleService ts( ResourceManager::instance()->serviceRegistry()->discoverTripleService() );

    StatementListIterator it( ts.listStatements( KMetaData::defaultGraph(), 
						 Statement( m_uri, Node(), Node() ) ), 
			      &ts );
    while( it.hasNext() ) {
      const Statement& s = it.next();

      // load the type
      if( s.predicate.value == KMetaData::typePredicate() ) {
	if( m_type.isEmpty() )
	  m_type = s.object.value;
      }
      else {
	PropertiesMap::iterator oldProp = m_properties.find( s.predicate.value );
	if( s.object.type == NodeResource ) {
	  if( !loadProperty( s.predicate.value, Resource( s.object.value ) ) )
	    return false;
	}
	else {
	  if( !loadProperty( s.predicate.value, KMetaData::RDFLiteralToValue( s.object.value ) ) )
	    return false;
	}
      }
    }
   
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


bool Nepomuk::KMetaData::ResourceData::save()
{
  if( m_initialized ) {
    TripleService ts( ResourceManager::instance()->serviceRegistry()->discoverTripleService() );

    // make sure our graph exists
    // ==========================
    if( !ts.listGraphs().contains( KMetaData::defaultGraph() ) )
      if( ts.addGraph( KMetaData::defaultGraph() ) ) {
	ResourceManager::instance()->notifyError( m_uri, ERROR_COMMUNICATION );
	return false;
      }

    // remove everything about this resource from the store (FIXME: better and faster syncing)
    // =======================================================================================
    if( ts.removeAllStatements( KMetaData::defaultGraph(), Statement( m_uri, Node(), Node() ) ) ) {
      ResourceManager::instance()->notifyError( m_uri, ERROR_COMMUNICATION );
      return false;
    }

    // save the type
    // =============
    if( ts.addStatement( KMetaData::defaultGraph(), 
			 Statement( Node(m_uri), Node(KMetaData::typePredicate()), Node(m_type) ) ) ) {
      ResourceManager::instance()->notifyError( m_uri, ERROR_COMMUNICATION );
      return false;
    }

    // save the properties
    // ===================
    for( PropertiesMap::const_iterator it = m_properties.constBegin();
	 it != m_properties.constEnd(); ++it ) {

      if( it.value().second & Deleted )
	continue;

      QString predicate = it.key();
      const Variant& val = it.value().first;

      // one-to-one Resource
      if( val.isResource() ) {
	if( ts.addStatement( KMetaData::defaultGraph(), 
			     Statement( m_uri, predicate, val.toResource().uri() ) ) ) {
	  ResourceManager::instance()->notifyError( m_uri, ERROR_COMMUNICATION );
	  return false;
	}
      }

      // one-to-many Resource
      else if( val.isResourceList() ) {
	const QList<Resource>& l = val.toResourceList();
	for( QList<Resource>::const_iterator resIt = l.constBegin(); resIt != l.constEnd(); ++resIt ) {
	  if( ts.addStatement( KMetaData::defaultGraph(), 
			       Statement( m_uri, predicate, (*resIt).uri() ) ) ) {
	    ResourceManager::instance()->notifyError( m_uri, ERROR_COMMUNICATION );
	    return false;
	  }
	}
      }

      // one-to-many literals
      else if( val.isList() ) {
	QStringList l = KMetaData::valuesToRDFLiterals( val );
	for( QStringList::const_iterator valIt = l.constBegin(); valIt != l.constEnd(); ++valIt ) {
	  if( ts.addStatement( KMetaData::defaultGraph(), 
			       Statement( m_uri, predicate, Node( *valIt, NodeLiteral ) ) ) ) {
	    ResourceManager::instance()->notifyError( m_uri, ERROR_COMMUNICATION );
	    return false;
	  }
	}
      }

      // one-to-one literal
      else {
	if( ts.addStatement( KMetaData::defaultGraph(), 
			     Statement( m_uri, 
					predicate, 
					Node( KMetaData::valueToRDFLiteral( val ), NodeLiteral ) ) ) ) {
	  ResourceManager::instance()->notifyError( m_uri, ERROR_COMMUNICATION );
	  return false;
	}
      }
    }

    return true;
  }
  else
    return false;
}


/**
 * Merge in changes from the local store
 */
bool Nepomuk::KMetaData::ResourceData::merge()
{
  // TODO: do more intelligent syncing
  ResourceData* currentData = new ResourceData( m_uri );
    
  if( currentData->load() ) {
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
    return true;
  }
    
  delete currentData;
  return false;
}


/**
 * Compares the properties of two ResourceData objects taking into account the Deleted flag
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

  for( PropertiesMap::const_iterator it = other.m_properties.constBegin();
       it != other.m_properties.constEnd(); ++it ) {
    PropertiesMap::const_iterator it2 = this->m_properties.constFind( it.key() );

    // 1. the property does not exist here
    if( it2 == this->m_properties.constEnd() )
      return false;
    // 2. the values differ or it has been removed here
    if( it.value().first != it2.value().first ||
	it2.value().second & Deleted )
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
  ResourceIdService resids( ResourceManager::instance()->serviceRegistry()->discoverResourceIdService() );
  QString uniqueUri = resids.toUniqueUrl( uri );
  if( uniqueUri.isEmpty() ) {
    qDebug() << "(ResourceData) determining unique URI failed. Falling back to plain URI." << endl;
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



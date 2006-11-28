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

#include <knep/knep.h>
#include <knep/services/tripleservice.h>

using namespace Nepomuk::Backbone::Services;
using namespace Nepomuk::Backbone::Services::RDF;

QHash<QString, Nepomuk::KMetaData::ResourceData*> Nepomuk::KMetaData::ResourceData::s_data;


Nepomuk::KMetaData::ResourceData::ResourceData( const QString& uri_, const QString& type_ )
  : uri( uri_ ),
    type( type_ ),
    m_ref(1),
    m_initialized( false )
{
}


Nepomuk::KMetaData::ResourceData::~ResourceData()
{
  s_data.remove( uri );
}


bool Nepomuk::KMetaData::ResourceData::init()
{
  if( !m_initialized )
    m_initialized = load();
  return m_initialized;
}


bool Nepomuk::KMetaData::ResourceData::exists() const
{
  TripleService ts( ResourceManager::instance()->serviceRegistry()->discoverTripleService() );
  
  // the resource has to exists either as a subject or as an object
  return( !ts.contains( "FIXME: proper graph id", Statement( uri, Node(), Node() ) ) || 
	  !ts.contains( "FIXME: proper graph id", Statement( Node(), Node(), uri ) ) );
}


/**
 * \return false if the resource does not exist in the local NEPOMUK DB.
 */
bool Nepomuk::KMetaData::ResourceData::load()
{
  // FIXME: load everything from the store and mark it as non-modified
  // Do not load recursively. Instead load other resources on first access.
  return false;
}


bool Nepomuk::KMetaData::ResourceData::save()
{
  // FIXME: save everything back to the store.
  //        Should we make this recursive or not?
  // problem: what if it was created in the meantime but has another type?
  return false;
}


/**
 * Merge in changes from the local store
 */
bool Nepomuk::KMetaData::ResourceData::merge()
{
  // TODO: do more intelligent syncing
  ResourceData* currentData = new ResourceData( uri );
    
  if( currentData->load() ) {
    // merge in possible remote changes
    for( PropertiesMap::const_iterator it = currentData->properties.constBegin();
	 it != currentData->properties.constEnd(); ++it ) {

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

      PropertiesMap::iterator it2 = this->properties.find( it.key() );

      if( it2 != this->properties.constEnd() ) {
	if( !(it2.value().second & Modified) ) {
	  it2.value().first = it.value().first;
	}
      }
      else {
	this->properties.insert( it.key(), it.value() );
      }
    }

    PropertiesMap::iterator it = this->properties.begin();
    while( it != this->properties.end() ) {

      // 4. a value that exists here does not exist there (second for-loop?)

      // 4.1 it has been modified here
      //     -> keep it

      // 4.2 it has not been modified here
      //     -> remove it since it has been deleted in the meantime

      PropertiesMap::const_iterator it2 = currentData->properties.constFind( it.key() );
      if( it2 == currentData->properties.constEnd() &&
	  !(it.value().second & Modified) ) {
	it = this->properties.erase( it );
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

  if( this->uri != other.uri ||
      this->type != other.type )
    return false;

  // Evil is among us!
  const_cast<ResourceData*>(this)->init();
  const_cast<ResourceData*>(&other)->init();

  for( PropertiesMap::const_iterator it = other.properties.constBegin();
       it != other.properties.constEnd(); ++it ) {
    PropertiesMap::const_iterator it2 = this->properties.constFind( it.key() );

    // 1. the property does not exist here
    if( it2 == this->properties.constEnd() )
      return false;
    // 2. the values differ or it has been removed here
    if( it.value().first != it2.value().first ||
	it2.value().second & Deleted )
      return false;
  }

  for( PropertiesMap::const_iterator it = this->properties.constBegin();
       it != this->properties.constEnd(); ++it ) {
    // 3. the property does not exist there
    if( other.properties.constFind( it.key() ) == this->properties.constEnd() )
      return false;
    // 4. the values differ (already handled above)
  }

  return true;
}


Nepomuk::KMetaData::ResourceData* Nepomuk::KMetaData::ResourceData::data( const QString& uri, const QString& type )
{
  QHash<QString, ResourceData*>::iterator it = s_data.find( uri );
  if( it == s_data.end() ) {
    ResourceData* d = new ResourceData( uri, type );

    s_data[uri] = d;

    return d;
  }
  else {
    return it.value();
  }
}



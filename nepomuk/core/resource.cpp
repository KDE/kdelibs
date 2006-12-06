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

#include "resource.h"
#include "resourcedata.h"
#include "resourcemanager.h"


Nepomuk::KMetaData::Resource::Resource()
{
  // invalid data
  d = new ResourceData();
  d->ref();
}


Nepomuk::KMetaData::Resource::Resource( const Nepomuk::KMetaData::Resource::Resource& res )
{
  d = res.d;
  d->ref();
}


Nepomuk::KMetaData::Resource::Resource( const QString& uri, const QString& type )
{
  d = ResourceData::data( uri, type );
  d->ref();
}


Nepomuk::KMetaData::Resource::~Resource()
{
  if( !d->deref() ) {
    if( ResourceManager::instance()->autoSync() && modified() )
      sync();
    d->deleteData();
  }
}


Nepomuk::KMetaData::Resource& Nepomuk::KMetaData::Resource::operator=( const Resource& res )
{
  if( d != res.d ) {
    if( !d->deref() ) {
      if( ResourceManager::instance()->autoSync() && modified() )
	sync();
      d->deleteData();
    }
    d = res.d;
    d->ref();
  }

  return *this;
}


const QString& Nepomuk::KMetaData::Resource::uri() const
{
  return d->uri;
}


const QString& Nepomuk::KMetaData::Resource::type() const
{
  d->init();
  return d->type;
}


QString Nepomuk::KMetaData::Resource::className() const
{
  return type().section( '#', -1 );
}


int Nepomuk::KMetaData::Resource::sync()
{
  // FIXME: inform the manager about failures

  d->init();

  if( d->merge() )
    if( d->save() )
      return 0;

  return -1;
}


Nepomuk::KMetaData::Variant Nepomuk::KMetaData::Resource::getProperty( const QString& uri ) const
{
  d->init();

  ResourceData::PropertiesMap::iterator it = d->properties.find( uri );
  if( it != d->properties.end() )
    if( !( it.value().second & ResourceData::Deleted ) )
      return it.value().first;
  
  return Variant();
}


void Nepomuk::KMetaData::Resource::setProperty( const QString& uri, const Nepomuk::KMetaData::Variant& value )
{
  d->init();

  // mark the value as modified
  d->properties[uri] = qMakePair<Variant, int>( value, ResourceData::Modified );
}


void Nepomuk::KMetaData::Resource::removeProperty( const QString& uri )
{
  d->init();

  ResourceData::PropertiesMap::iterator it = d->properties.find( uri );
  if( it != d->properties.end() )
    it.value().second = ResourceData::Modified|ResourceData::Deleted;
}


bool Nepomuk::KMetaData::Resource::modified() const
{
  for( ResourceData::PropertiesMap::const_iterator it = d->properties.constBegin();
       it != d->properties.constEnd(); ++it )
    if( it.value().second & ResourceData::Modified )
      return true;

  return false;
}


bool Nepomuk::KMetaData::Resource::inSync() const
{
  return d->inSync();
}


bool Nepomuk::KMetaData::Resource::exists() const
{
  return d->exists();
}


bool Nepomuk::KMetaData::Resource::isValid() const
{
  return d->isValid();
}


bool Nepomuk::KMetaData::Resource::operator==( const Resource& other ) const
{
  if( this == &other )
    return true;

  return( *d == *other.d );
}

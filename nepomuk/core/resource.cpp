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
#include "ontology.h"
#include "tools.h"

#include <klocale.h>
#include <kdebug.h>


Nepomuk::KMetaData::Resource::Resource()
{
  // invalid data
  m_data = new ResourceData();
  m_data->ref();
}


Nepomuk::KMetaData::Resource::Resource( const Nepomuk::KMetaData::Resource::Resource& res )
{
  m_data = res.m_data;
  m_data->ref();
}


Nepomuk::KMetaData::Resource::Resource( const QString& uri, const QString& type )
{
  m_data = ResourceData::data( uri, type );
  m_data->ref();
}


Nepomuk::KMetaData::Resource::~Resource()
{
  if( !m_data->deref() ) {
    if( ResourceManager::instance()->autoSync() && modified() )
      sync();
    m_data->deleteData();
  }
}


Nepomuk::KMetaData::Resource& Nepomuk::KMetaData::Resource::operator=( const Resource& res )
{
  if( m_data != res.m_data ) {
    if( !m_data->deref() ) {
      if( ResourceManager::instance()->autoSync() && modified() )
	sync();
      m_data->deleteData();
    }
    m_data = res.m_data;
    m_data->ref();
  }

  return *this;
}


const QString& Nepomuk::KMetaData::Resource::uri() const
{
  return m_data->uri();
}


const QString& Nepomuk::KMetaData::Resource::type() const
{
  m_data->init();
  return m_data->type();
}


QString Nepomuk::KMetaData::Resource::className() const
{
  return type().section( QRegExp( "[#:]" ), -1 );
}


int Nepomuk::KMetaData::Resource::sync()
{
  // FIXME: inform the manager about failures

  m_data->init();

  if( m_data->merge() )
    if( m_data->save() )
      return 0;

  return -1;
}


QHash<QString, Nepomuk::KMetaData::Variant> Nepomuk::KMetaData::Resource::allProperties() const
{
  m_data->init();
  return m_data->allProperties();
}


bool Nepomuk::KMetaData::Resource::hasProperty( const QString& uri ) const
{
  m_data->init();
  return m_data->hasProperty( ensureNamespace( uri ) );
}


Nepomuk::KMetaData::Variant Nepomuk::KMetaData::Resource::getProperty( const QString& uri ) const
{
  m_data->init();
  return m_data->getProperty( ensureNamespace( uri ) );
}


void Nepomuk::KMetaData::Resource::setProperty( const QString& uri, const Nepomuk::KMetaData::Variant& value )
{
  m_data->init();
  m_data->setProperty( ensureNamespace( uri ), value );
}


void Nepomuk::KMetaData::Resource::removeProperty( const QString& uri )
{
  m_data->init();
  m_data->removeProperty( ensureNamespace( uri ) );
}


void Nepomuk::KMetaData::Resource::remove()
{
  m_data->init();
  m_data->remove();
}


void Nepomuk::KMetaData::Resource::revive()
{
  m_data->revive();
}


bool Nepomuk::KMetaData::Resource::isProperty( const QString& uri ) const
{
  return !ResourceManager::instance()->allResourcesWithProperty( ensureNamespace( uri ), *this ).isEmpty();
}


bool Nepomuk::KMetaData::Resource::modified() const
{
  return m_data->modified();
}


bool Nepomuk::KMetaData::Resource::inSync() const
{
  return m_data->inSync();
}


bool Nepomuk::KMetaData::Resource::exists() const
{
  return m_data->exists();
}


bool Nepomuk::KMetaData::Resource::isValid() const
{
  return m_data->isValid();
}


bool Nepomuk::KMetaData::Resource::operator==( const Resource& other ) const
{
  if( this == &other )
    return true;

  return( *m_data == *other.m_data );
}


QString Nepomuk::KMetaData::errorString( int code )
{
  switch( code ) {
  case ERROR_SUCCESS:
    return i18n("Success");
  case ERROR_COMMUNICATION:
    return i18n("Communication error");
  case ERROR_INVALID_TYPE:
    return i18n("Invalid type in Database");
  default:
    return i18n("Unknown error");
  }
}

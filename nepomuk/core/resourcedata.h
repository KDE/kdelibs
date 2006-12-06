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

#ifndef _NEPOMUK_KMETADATA_RESOURCE_DATA_H_
#define _NEPOMUK_KMETADATA_RESOURCE_DATA_H_

#include <QtCore>

#include <variant.h>


namespace Nepomuk {
  namespace KMetaData {
    class ResourceData
      {
      public:
	ResourceData( const QString& uri_ = QString(), const QString& type_ = QString() );

	/**
	 * Uses instead of the destructor in Resource
	 */
	void deleteData();

	inline int ref() {
	  return ++m_ref;
	}

	inline int deref() {
	  return --m_ref;
	}

	enum Flags {
	  Modified = 0x2,
	  Deleted = 0x4
	};

	typedef QHash<QString, QPair<Variant, int> > PropertiesMap;
  
	/**
	 * map of all properties including a flag field
	 */
	PropertiesMap properties;

	bool exists() const;

	bool isValid() const;

	bool inSync();

	/**
	 * Initializes the data object, i.e. loads it for the first time.
	 * Does nothing on subsequent calls.
	 */
	bool init();

	/**
	 * Load all properties stored in the local Nepomuk DB that have subject \a uri
	 * into this object.
	 *
	 * \return false if the resource does not exist in the local NEPOMUK DB.
	 */
	bool load();

	/**
	 * Save all properties and the type back into the local NEPOMUK DB overwriting
	 * any existing entries.
	 * Entries in the DB that do not exist in this object will be removed.
	 *
	 * Use merge() to preserve non-existing propreties.
	 */
	bool save();

	/**
	 * Merge in changes from the local store
	 */
	bool merge();

	/**
	 * Compares the properties of two ResourceData objects taking into account the Deleted flag
	 */
	bool operator==( const ResourceData& other ) const;

	/**
	 * The KMetaData lib is based on the fact that for each uri only one ResourceData object is
	 * created at all times. This method searches for an existing data object to reuse or creates
	 * a new one if none exists.
	 *
	 * The Resource constructors use this method in combination with ref()
	 */
	static ResourceData* data( const QString& uri, const QString& type );

	QString uri;
	QString type;

      private:
	~ResourceData();
	bool loadProperty( const QString& name, const Variant& val );

	int m_ref;
	bool m_initialized;

	static QHash<QString, ResourceData*> s_data;
	static QString s_defaultType;

	friend class ResourceManager;
      };
  }
}

#endif

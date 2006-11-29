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
	~ResourceData();

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

	/**
	 * Initializes the data object, i.e. loads it for the first time.
	 * Does nothing on subsequent calls.
	 */
	bool init();

	/**
	 * \return false if the resource does not exist in the local NEPOMUK DB.
	 */
	bool load();

	bool save();

	/**
	 * Merge in changes from the local store
	 */
	bool merge();

	/**
	 * Compares the properties of two ResourceData objects taking into account the Deleted flag
	 */
	bool operator==( const ResourceData& other ) const;

	static ResourceData* data( const QString& uri, const QString& type );

	QString uri;
	QString type;

      private:
	int m_ref;
	bool m_initialized;

	static QHash<QString, ResourceData*> s_data;
	static QString s_defaultType;

	friend class ResourceManager;
      };
  }
}

#endif

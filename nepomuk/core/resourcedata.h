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

#include <knep/services/rdf.h>


namespace Nepomuk {
  namespace KMetaData {
    class ResourceData
      {
      public:
	ResourceData( const QString& uri_ = QString(), const QString& type_ = QString() );

	/**
	 * Used instead of the destructor in Resource. The reason for the existance of
	 * this method is that the destructor does not remove the uri from the global
	 * data map. That behaviour is necessary since in certain situations temporary
	 * ResourceData instances are created.
	 */
	void deleteData();

	inline int ref() {
	  return ++m_ref;
	}

	inline int deref() {
	  return --m_ref;
	}

	inline int cnt() const {
	  return m_ref;
	}

	enum Flags {
	  Modified = 0x2, /*< The resource or property has locally been modified */
	  Deleted = 0x4, /*< The resource has actually been deleted in a sync operation */
	  Removed = 0x8, /*< The resource or property has been scheduled for removal */
	  Syncing = 0x10 /*< The resource is currently being synced */
	};

	const QString& uri() const;
	const QString& type() const;

	QHash<QString, Variant> allProperties() const;

	bool hasProperty( const QString& uri ) const;

	Variant getProperty( const QString& uri ) const;

	void setProperty( const QString& uri, const Variant& value );

	void removeProperty( const QString& uri );

	/**
	 * Remove this resource data. The data is not deleted locally or remotly
	 * until save() is called.
	 */
	void remove();

	void revive();

	bool modified() const;

	bool removed() const;

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
	 *
	 * This method will save directly and uncached to the store. It is recommended to
	 * rely on the cached syncing that the ResourceManger provides.
	 *
	 * Be aware that calling save will not interfere with any syncing operation started
	 * via startSync.
	 *
	 * \sa allStatements
	 */
	bool save();

	/**
	 * Merge in changes from the local store
	 */
	bool merge();

	/**
	 * Start a thread-safe sync operation.
	 * Use this method to mark the state of the resource data to be the one being
	 * synced back.
	 *
	 * Subsequent calls will block until the first call has been released via endSync
	 *
	 * \sa endSync
	 */
	void startSync();

	/**
	 * Finish a thread-safe sync operation started with startSync
	 *
	 * \param updateFlags If true the resource flags will be updated, i.e. synced properties
	 *                    will be marked as not-modified. In general this should be set true
	 *                    if the sync was successful.
	 */
	void endSync( bool updateFlags = true );

	/**
	 * Generates a list of all RDF statements this Resource data object currently represents.
	 * \param flags A filter to be used. Only those properties that match flags are returned.
	 */
	QList<Backbone::Services::RDF::Statement> allStatements( int flags ) const;

	/**
	 * \return A list of all statements that have to be added to the store in a sync. This does not
	 * include those statements that already exist in the store.
	 */
	QList<Backbone::Services::RDF::Statement> allStatementsToAdd() const;

	/**
	 * \return a list of all statements that need to be removed from the store in a sync, i.e. those
	 * properties that have been removed. In case that the whole resource has been removed it is 
	 * recommended to not use this method but do a plain removal of all statements related to this 
	 * resource.
	 */
	QList<Backbone::Services::RDF::Statement> allStatementsToRemove() const;

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

      private:
	~ResourceData();
	bool loadProperty( const QString& name, const Variant& val );

	typedef QHash<QString, QPair<Variant, int> > PropertiesMap;
  
	/**
	 * map of all properties including a flag field
	 */
	PropertiesMap m_properties;

	QString m_uri;
	QString m_type;

	int m_flags;

	int m_ref;
	bool m_initialized;

	QMutex m_syncingMutex;
	QMutex m_modificationMutex;

	static QHash<QString, ResourceData*> s_data;
	static QString s_defaultType;

	friend class ResourceManager;
      };
  }
}

#endif

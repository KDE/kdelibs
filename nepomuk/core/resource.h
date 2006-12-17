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

#ifndef _NEPOMUK_KMETADATA_RESOURCE_H_
#define _NEPOMUK_KMETADATA_RESOURCE_H_

#include <QtCore>

#include <kmetadata/kmetadata_export.h>


namespace Nepomuk {
  namespace KMetaData {

    class ResourceData;
    class Variant;

    enum ErrorCode {
      ERROR_SUCCESS = 0,
      ERROR_COMMUNICATION, /**< A commication error, i.e. libKNepCLient connection failure */
      ERROR_INVALID_TYPE
    };

    /**
     * \return A human-readble string.
     */
    // FIXME: add the uri of the resource as parameter
    QString errorString( int code );

    /**
     * \brief Resource is the central object type in libKMetaData. It represents a piece of
     * information of any kind.
     *
     * Resources are identified by their unique URI (which 
     * correlates directly with the URI in the local NEPOMUK RDF storage.
     *
     * Resource objects with the same URI share their data.
     *
     * Normally they are also synced with the local storage automatically.
     *
     * See \ref hacking for details on how to use Resource.
     *
     * \see ResourceManager
     */
    class KMETADATA_EXPORT Resource
      {
      public:
	/**
	 * Creates an empty invalid Resource
	 */
	Resource();

	Resource( const Resource& );

	/**
	 * Creates a new Resource object representing the data referenced by \a uri
	 * which is of type \a type. If the uri already exists the type is ignored and
	 * the existing data's type is used.
	 *
	 * The actual resource data is loaded on demand. Thus, it is possible to work
	 * with Resources as if they were in memory all the time.
	 *
	 * \param uri The URI identifying the resource in question. This might also be 
	 *            a file path.
	 * \param type The URI identifying the type of the resource. If it is empty
	 *             Resource falls back to http://www.w3.org/2000/01/rdf-schema\#Resource or
	 *             in case the resource already exists the type will be read from the 
	 *             store.
	 */
	Resource( const QString& uri, const QString& type = QString() );
	virtual ~Resource();

	Resource& operator=( const Resource& );

	/**
	 * The URI of the resource, uniquely identifying it.
	 */
	const QString& uri() const;

	/**
	 * The type URI.
	 * \sa name()
	 */
	const QString& type() const;

	/**
	 * The name of the class this Resource represents an object of.
	 * 
	 * \sa type()
	 */
	QString className() const;

	/**
	 * Resync the data with the local storage. Unless autosync is disabled there
	 * is normally no need to call this explicitely.
	 *
	 * \return Some error code which still has to be defined.
	 *
	 * \sa ResourceManager::setAutoSync
	 */
	int sync();

	/**
	 * \return A list of all defined properties
	 */
	QHash<QString, Variant> allProperties() const;

	/**
	 * Retrieve the value of property \a uri. If the property is not defined for
	 * this resource an invalid, empty Variant object is returned.
	 */
	Variant getProperty( const QString& uri ) const;

	/**
	 * Set a property of the resource.
	 * \param uri The URI which describes the property (i.e. the RDF predicate name)
	 * \param value The value of the property (i.e. the object of the RDF triple)
	 *
	 * This method only changes the resource locally. The new data is not written
	 * back to the Nepomuk store before a call to sync().
	 */
	void setProperty( const QString& uri, const Variant& value );

	/**
	 * Remove property \a uri from this resource object.
	 *
	 * This method only changes the resource locally. The new data is not written
	 * back to the Nepomuk store before a call to sync().
	 */
	void removeProperty( const QString& uri );

	/**
	 * \return true if the data in this object has been modified and not yet
	 * synced with the local NEPOMUK RDF store.
	 */
	bool modified() const;

	/**
	 * \return true if this object represents the exact data from the local NEPOMUK RDF 
	 * store. Differs from modified() in that it also handles changes in the store that
	 * have not been loaded yet.
	 *
	 * \sa exists()
	 */
	bool inSync() const;

	/**
	 * \return true if this resource (i.e. the uri of this resource) exists in the local
	 * NEPOMUK RDF store, either as subject or as object.
	 *
	 * \sa inSync()
	 */
	bool exists() const;

	/**
	 * \return true if this Resource object is valid, i.e. has a proper URI and type and
	 * can be synced with the local NEPOMUK RDF store.
	 */
	bool isValid() const;

	/**
	 * Operator to compare two Resource objects. Normally one does not need this. It is
	 * mainly intended for testing and debugging purposes.
	 */
	bool operator==( const Resource& ) const;
	
      private:
	ResourceData* m_data;

	class Private;
	Private* d; // unused
      };
  }
}

#endif

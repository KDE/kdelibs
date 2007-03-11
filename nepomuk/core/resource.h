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

OTHERCLASSES

    enum ErrorCode {
      ERROR_SUCCESS = 0,
      ERROR_COMMUNICATION, /**< A commication error, i.e. libKNepCLient connection failure */
      ERROR_INVALID_TYPE
    };

    /**
     * \return A human-readble string.
     */
    // FIXME: add the uri of the resource as parameter
    KMETADATA_EXPORT QString errorString( int code );

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
     * All methods in Resource are thread-safe.
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
	 * \param uriOrIdentifier The unique URI or an arbitrary identifier of the resource. 
	 *                        If the URI exists in the RDF store it is used to load the 
	 *                        related properties. If not the passed string is treated
	 *                        as an identifier.
	 *                        If a resource exists in the store which has this identifier
	 *                        set this resource's properties are loaded. Otherwise the
	 *                        resource is created in the store
	 *                        with a new random URI which can be accessed through \a uri
	 *                        after the resource has been synced. The resource can later
	 *                        again be found through the same identifier.
	 *                        
	 * \param type The URI identifying the type of the resource. If it is empty
	 *             Resource falls back to http://www.w3.org/2000/01/rdf-schema\#Resource or
	 *             in case the resource already exists the type will be read from the 
	 *             store.
	 *
	 * Example:
	 *
	 * The best way to understand the URI and identifier system is through file resources.
	 * When a Resource object is created with the local path of the file as an identifier:
	 *
	 * \code
	 * Resource myfile( "/tmp/testfile.txt" );
	 * \endcode
	 *
	 * Now the URI of the resource in the store representing metadata for the file /tmp/testfile.txt
	 * is referred to by myfile.uri() which differs from the path of the file. However, the path of
	 * the file is saved as a \a hasIdentifier relation which means that it can be used to easily find
	 * the related resource.
	 */
	Resource( const QString& uriOrIdentifier, const QString& type = QString() );
	virtual ~Resource();

	Resource& operator=( const Resource& );

	/**
	 * The URI of the resource, uniquely identifying it. This URI in most
	 * cases is a virtual one which has been created from a generic base
	 * namespace and some identifier.
	 *
	 * the most important thing to remember is that the URI of for example
	 * a file has no relation to its local path.
	 *
	 * In case the resource has not been synced yet the URI may be empty.
	 *
	 * \sa getIdentifiers
	 */
	const QString& uri() const;

	/**
	 * The type URI.
	 * \sa name()
	 */
	const QString& type() const;

	/**
	 * The name of the class this Resource represents an object of.
	 * The classname is derived from the type URI (see Resource::uri).
	 * For a translated user readable name of the resource see
	 * Ontology::typeName.
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
	 * Check if property identified by \a uri is defined
	 * for this resource.
	 *
	 * Be aware that this method does not check if the property
	 * is defined for this resource's type!
	 *
	 * \param uri The URI identifying the property. If this URI does
	 *            not include a namespace the default namespace is
	 *            prepended.
	 *
	 * \return true if property \a uri has a value set.
	 *
	 * \sa Ontology::defaultNamespace
	 */
	bool hasProperty( const QString& uri ) const;

	/**
	 * Retrieve the value of property \a uri. If the property is not defined for
	 * this resource an invalid, empty Variant object is returned.
	 *
	 * \param uri The URI identifying the property. If this URI does
	 *            not include a namespace the default namespace is
	 *            prepended.
	 *
	 * \sa Ontology::defaultNamespace
	 */
	Variant getProperty( const QString& uri ) const;

	/**
	 * Set a property of the resource.
	 *
	 * \param uri The URI identifying the property. If this URI does
	 *            not include a namespace the default namespace is
	 *            prepended.
	 * \param value The value of the property (i.e. the object of the RDF triple)
	 *
	 * This method only changes the resource locally. The new data is not written
	 * back to the Nepomuk store before a call to sync().
	 *
	 * Calling setProperty will revert any previous calls to remove.
	 *
	 * \sa Ontology::defaultNamespace
	 */
	void setProperty( const QString& uri, const Variant& value );

	/**
	 * Remove property \a uri from this resource object.
	 *
	 * This method only changes the resource locally. The new data is not written
	 * back to the Nepomuk store before a call to sync().
	 *
	 * \param uri The URI identifying the property. If this URI does
	 *            not include a namespace the default namespace is
	 *            prepended.
	 *
	 * \sa Ontology::defaultNamespace
	 */
	void removeProperty( const QString& uri );

	/**
	 * Remove this resource completely.
	 *
	 * The resource will only be marked as deleted locally and not be removed from
	 * the local Nepomuk RDF store until a call to sync.
	 *
	 * One call to setProperty will revert the deletion of a resource.
	 *
	 * \sa revive
	 */
	void remove();

	/**
	 * Revive a previously removed resource. If the resource was not yet synced all
	 * properties (except those that have actually been removed via removeProperty)
	 * are restored to their previous values.
	 *
	 * \sa remove
	 */
	void revive();

	/**
	 * Check if this resource is a property of type \a uri, i.e. if there is some
	 * other resource that has this resource as a property of type \a uri.
	 *
	 * \return true if there exists a resource with property \a uri set to this resource.
	 *
	 * \sa ResourceManager::allResourcesWithProperty
	 */
	bool isProperty( const QString& uri ) const;

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
	 * This method ignores any previous unsynced remove operations and only checks for 
	 * the presence of the resource in the local RDF store.
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

METHODS
      private:
	ResourceData* m_data;

	class Private;
	Private* d; // unused

	friend class ResourceData;
      };
  }
}

#endif

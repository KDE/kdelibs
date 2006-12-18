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

#ifndef _NEPOMUK_RESOURCE_MANAGER_H_
#define _NEPOMUK_RESOURCE_MANAGER_H_

#include <kmetadata/kmetadata_export.h>

#include <QtCore>


namespace Nepomuk {
  namespace Backbone {
    class Registry;
  }

  namespace KMetaData {

    class Resource;
    class Ontology;

    /**
     * \brief The ResourceManager is the central \a %KMetaData configuration point.
     *
     * For now it only provides the possibility to disbable auto syncing and get informed 
     * of Resource changes via the resourceModified signal.
     *
     * At the moment auto syncing is not complete and will only write back changes once 
     * all instances of a Resource have been deleted. In the future auto syncing will try
     * to keep in sync with external changes also.
     */
    class KMETADATA_EXPORT ResourceManager : public QObject
      {
	Q_OBJECT

      public:
	~ResourceManager();

	static ResourceManager* instance();

	/**
	 * The main purpose of the init method is to check for errors in the initialization.
	 * There is no real need to call it before using Resource.
	 * It checks if the NEPOMUK-KDE Registry is running and provides the RDF storage services
	 * used by libKMetaData.
	 *
	 * \return 0 if all necessary components could be found and -1 otherwise.
	 *
	 * FIXME: introduce error codes and human readable translated error messages.
	 */
	int init();

	/**
	 * The Ontology instance representing the underlying Nepomuk desktop ontology.
	 */
	Ontology* ontology() const;

	/**
	 * The NEPOMUK Service Registry used.
	 */
	Backbone::Registry* serviceRegistry() const;

	/**
	 * \return true if autosync is enabled
	 */
	bool autoSync() const;

	/**
	 * Creates a Resource object representing the data referenced by \a uri.
	 * The result is the same as from using the Resource::Resource( const QString&, const QString& )
	 * constructor with an empty type.
	 *
	 * \return The Resource representing the data at \a uri or an invalid Resource object if the local
	 * NEPOMUK RDF store does not contain an object with URI \a uri.
	 */
	Resource createResourceFromUri( const QString& uri );


	/**
	 * Retrieve a list of all resources of the specified \a type.
	 *
	 * This includes Resources that are not synced yet so it might
	 * not represent exactly the state as in the RDF store.
	 */
	QList<Resource> allResourcesOfType( const QString& type ) const;

	/**
	 * Non-public API. Used by Resource to signalize errors.
	 */
	KMETADATA_NO_EXPORT void notifyError( const QString& uri, int errorCode );

      Q_SIGNALS:
	/**
	 * This signal gets emitted whenever a Resource changes due to a sync procedure.
	 * Be aware that modifying resources locally via the Resource::setProperty method
	 * does not result in a resourceModified signal being emitted.
	 *
	 * \param uri The URI of the modified resource.
	 *
	 * NOT IMPLEMENTED YET
	 */
	void resourceModified( const QString& uri );

	/**
	 * Whenever a problem occurs (like for example failed resource syncing) this 
	 * signal is emitted.
	 *
	 * \param uri The resource related to the error.
	 * \param errorCode The type of the error (Resource::ErrorCode)
	 */
	void error( const QString& uri, int errorCode );

	// FIXME: add a loggin mechanism that reports successfully and failed sync operations and so on

      public Q_SLOTS:
	/**
	 * Enable or disable autosync. If autosync is enabled (which is the default)
	 * all Resource objects will be synced with the local storage automatically.
	 */
	void setAutoSync( bool enabled );

        /**
	 * Sync all Resource objects. There is no need to call this
	 * unless autosync has been disabled.
	 */
        void syncAll();

      private:
	ResourceManager();

	class Private;
	Private* d;
      };
  }
}

#endif

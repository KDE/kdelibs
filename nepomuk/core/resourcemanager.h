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

    /**
     * CAUTION: AutoSync is not implemented yet!
     */
    class KMETADATA_EXPORT ResourceManager : public QObject
      {
	Q_OBJECT

      public:
	~ResourceManager();

	static ResourceManager* instance();

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

      Q_SIGNALS:
	/**
	 * This signal gets emitted whenever a Resource changes due to a sync procedure.
	 * Be aware that modifying resources locally via the Resource::setProperty method
	 * does not result in a resourceModified signal being emitted.
	 */
	void resourceModified( const Resource& );

      public Q_SLOTS:
	/**
	 * Enable or disable autosync. If autosync is enabled (which is the default)
	 * all Resource objects wil be synced with the local storage automatically.
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

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

#ifndef _NEPOMUK_DBUS_DUMMY_SERVICE_H_
#define _NEPOMUK_DBUS_DUMMY_SERVICE_H_

#include <knepomuk/servicepublisher.h>
#include <knepomuk/servicepublisherdbusinterface.h>
#include <knepomuk/knepomuk_export.h>

//krazy:excludeall=dpointer
namespace Nepomuk {
    namespace Backbone {
	namespace Services {
	    class KNEP_EXPORT DummyServicePublisher : public ServicePublisher
		{
		    Q_OBJECT

		public:
		    DummyServicePublisher( const QString& name, const QString& url );
		    virtual ~DummyServicePublisher();

		public Q_SLOTS:
		    virtual QString test() = 0;
		};
	}

	namespace DBus {
	    class KNEP_EXPORT DummyServicePublisherInterface : public ServicePublisherInterface
		{
		    Q_OBJECT
		    Q_CLASSINFO("D-Bus Interface", "org.semanticdesktop.nepomuk.services.DummyService")

		public:
		    DummyServicePublisherInterface( ServicePublisher* s );

		public Q_SLOTS:
		    QString test();
		};
	}
    }
}

#endif

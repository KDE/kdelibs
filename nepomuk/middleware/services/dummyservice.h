/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _NEPOMUK_DBUS_DUMMY_SERVICE_H_
#define _NEPOMUK_DBUS_DUMMY_SERVICE_H_

#include "servicepublisher.h"
#include "servicepublisherdbusinterface.h"
#include "nepomuk_export.h"

//krazy:excludeall=dpointer
namespace Nepomuk {
    namespace Middleware {
	namespace Services {
	    class NEPOMUK_MIDDLEWARE_EXPORT DummyServicePublisher : public ServicePublisher
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
	    class NEPOMUK_MIDDLEWARE_EXPORT DummyServicePublisherInterface : public ServicePublisherInterface
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

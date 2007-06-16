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

#ifndef _NEPOMUK_SERVICE_DESC_H_
#define _NEPOMUK_SERVICE_DESC_H_

#include "nepomuk_export.h"

#include <QtCore/QString>
#include <QtCore/QMetaType>

//krazy:excludeall=dpointer
namespace Nepomuk {
    namespace Middleware {
	/**
	 * \brief ServiceDesc describes a service in the \a %Nepomuk system.
	 *
	 * A ServiceDesc object unambiguously identifies a \a %Nepomuk service
	 * on the desktop. It is mainly used internally by Registry to communicate
	 * with the outside world, i.e. the Nepomuk-KDE service registry.
	 *
	 * \internal The ServiceDesc class is not intended to be used in the public
	 * API. Please refer to Service::url, Service::name, and Service::type.
	 */
	class NEPOMUK_MIDDLEWARE_EXPORT ServiceDesc
	{
	public:
	    QString name;
	    QString url;
	    QString type;
	};
    }
}

/**
 * We want to use ServiceDesc with QVariant, thus we need a QMetaType
 */
Q_DECLARE_METATYPE(Nepomuk::Middleware::ServiceDesc)
Q_DECLARE_METATYPE(QList<Nepomuk::Middleware::ServiceDesc>)

#endif

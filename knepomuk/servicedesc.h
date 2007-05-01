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

#ifndef _NEPOMUK_SERVICE_DESC_H_
#define _NEPOMUK_SERVICE_DESC_H_

#include <knepomuk/knepomuk_export.h>

#include <QtCore/QString>
#include <QtCore/QMetaType>

//krazy:excludeall=dpointer
namespace Nepomuk {
    namespace Backbone {
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
	class KNEPOMUK_EXPORT ServiceDesc
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
Q_DECLARE_METATYPE(Nepomuk::Backbone::ServiceDesc)
Q_DECLARE_METATYPE(QList<Nepomuk::Backbone::ServiceDesc>)

#endif

/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING.LIB" for the exact licensing terms.
 */

#ifndef _NEPOMUK_BACKBONE_SERVICES_RDF_H_
#define _NEPOMUK_BACKBONE_SERVICES_RDF_H_

#include <QtCore/QMetaType>
#include <QtCore/QStringList>

#include <soprano/node.h>
#include <knepomuk/knepomuk_export.h>

namespace Nepomuk {
    namespace RDF {

	/**
	 * A QueryResultTable represents a query result.
	 */
	class KNEPOMUK_EXPORT QueryResultTable //krazy:exclude=dpointer
	    {
	    public:
		QueryResultTable();
	    
		/**
		 * Storing column names separately for
		 * memory optimization
		 */
		QStringList columns;
		QList<QList<Soprano::Node> > rows;
	    };
    }
}

Q_DECLARE_METATYPE(Nepomuk::RDF::QueryResultTable)

#endif

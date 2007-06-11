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

#ifndef _NEPOMUK_BACKBONE_SERVICES_RDF_H_
#define _NEPOMUK_BACKBONE_SERVICES_RDF_H_

#include <QtCore/QMetaType>
#include <QtCore/QStringList>

#include <soprano/node.h>
#include "nepomuk_export.h"

namespace Nepomuk {
    namespace RDF {

	/**
	 * A QueryResultTable represents a query result.
	 */
	class NEPOMUK_EXPORT QueryResultTable //krazy:exclude=dpointer
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

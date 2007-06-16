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

#ifndef _NEPMUK_RDF_STATEMENT_LIST_ITERATOR_H_
#define _NEPMUK_RDF_STATEMENT_LIST_ITERATOR_H_

#include "nepomuk_export.h"

#include <soprano/statement.h>

namespace Nepomuk {
    namespace Services {
	class RDFRepository;
    }

    namespace RDF {
	/**
	 * \brief Java-style iterator for listStatement results.
	 *
	 * This iterator can be used to simply traverse all the results from a RDFRepository 
	 * statement listing query without caring about doing subsequent list calls.
	 *
	 * Once a StatementListIterator has been constructed the query id cannot be reused.
	 *
	 * Usage:
	 * \code
	 * RDFRepository rep( repService );
	 * Statement s( Node("someresource"), Node(), Node() );
	 * StatementListIterator it( rep.queryListStatements( "main", s ), &rep );
	 * while( it.hasNext() )
	 *   doSomething( it.next() );
	 * \endcode
	 */
	class NEPOMUK_MIDDLEWARE_EXPORT StatementListIterator
	    {
	    public:
		/**
		 * Creates a new StatementListIterator which iterates over all statements
		 * of the query referenced by \a id.
		 */
		StatementListIterator( int id, Services::RDFRepository* service );

		/**
		 * Closes the query id.
		 * \sa RDFRepository::closeQuery
		 */
		~StatementListIterator();

		/**
		 * \return True if there is another statement after the current one.
		 */
		bool hasNext() const;

		/**
		 * Get the next statement.
		 *
		 * \return The next statement or an empty statement if hasNext() returns
		 * false
		 */
		const Soprano::Statement& next() const;

	    private:
		class Private;
		Private* const d;
	    };
    }
}

#endif



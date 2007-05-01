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

#ifndef _NEPMUK_RDF_STATEMENT_LIST_ITERATOR_H_
#define _NEPMUK_RDF_STATEMENT_LIST_ITERATOR_H_

#include <knepomuk/knepomuk_export.h>

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
	class KNEPOMUK_EXPORT StatementListIterator
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



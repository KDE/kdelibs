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

#include "statementlistiterator.h"
#include "rdfrepository.h"
#include "rdfdbus.h"

#include <soprano/statement.h>

#include <QtCore/QList>

#include <kdebug.h>


class Nepomuk::RDF::StatementListIterator::Private
{
public:
    int listId;

    Services::RDFRepository* service;
    QList<Soprano::Statement> statementList;

    // we cache the current item
    Soprano::Statement currentStatement;

    // the iterator always points to the next item
    QList<Soprano::Statement>::const_iterator statementIt;
};


Nepomuk::RDF::StatementListIterator::StatementListIterator( int id, Services::RDFRepository* service )
    : d( new Private() )
{
    d->service = service;
    d->listId = id;
    d->statementIt = d->statementList.begin(); // and also end since it is empty
}


Nepomuk::RDF::StatementListIterator::~StatementListIterator()
{
    //  qDebug( "Called RDFRepository->finishListing to cleanup the cache" );
    if ( d->listId > 0 )
        d->service->closeQuery( d->listId );

    delete d;
}


bool Nepomuk::RDF::StatementListIterator::hasNext() const
{
    //
    // we need to fetch statements here because the only way to see if we fetched
    // all statements is by retrieving an empty list.
    //
    if( d->statementIt == d->statementList.constEnd() ) {
        if( d->listId > 0 ) {
            // fetch next batch of statements
            // FIXME: what about construct????
            d->statementList = d->service->fetchListStatementsResults( d->listId, 20 );
            d->statementIt = d->statementList.constBegin();

            // check if listing is finished
            if( d->statementList.isEmpty() )
                d->listId = 0;
        }
    }

    return ( d->statementIt != d->statementList.constEnd() );
}


const Soprano::Statement&
Nepomuk::RDF::StatementListIterator::next() const
{
    if( hasNext() ) {
        d->currentStatement = *d->statementIt;
        ++d->statementIt;
    }
    else {
        kDebug(300001) << "(StatementListIterator) Called StatementListIterator::next() when there was no next entry.";
        d->currentStatement = Soprano::Statement();
    }

    return d->currentStatement;
}

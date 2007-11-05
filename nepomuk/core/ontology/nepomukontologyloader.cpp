/* This file is part of the Nepomuk-KDE libraries
    Copyright (c) 2007 Sebastian Trueg <trueg@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "nepomukontologyloader.h"
#include "global.h"

#include <Soprano/Statement>
#include <Soprano/Model>
#include <Soprano/QueryResultIterator>
#include <Soprano/Client/DBusModel>
#include <Soprano/Client/DBusClient>

#include <QtCore/QDebug>


class Nepomuk::NepomukOntologyLoader::Private
{
public:
    Private()
        : client("org.kde.NepomukServer") {
    }
    Soprano::Client::DBusClient client;
};


Nepomuk::NepomukOntologyLoader::NepomukOntologyLoader()
    : OntologyLoader(),
      d( new Private() )
{
}


Nepomuk::NepomukOntologyLoader::~NepomukOntologyLoader()
{
    delete d;
}


QList<Soprano::Statement> Nepomuk::NepomukOntologyLoader::loadOntology( const QUrl& uri )
{
    QList<Soprano::Statement> sl;

    if ( Soprano::Model* model = d->client.createModel( "main" ) ) {
        // get the complete named graph describing the ontology
        Soprano::QueryResultIterator it = model->executeQuery( QString( "construct {?s ?p ?o} "
                                                                        "where { GRAPH <%1> { ?s ?p ?o } . }" )
                                                               .arg( uri.toString() ),
                                                               Soprano::Query::QUERY_LANGUAGE_SPARQL );
        while ( it.next() ) {
            sl.append( it.currentStatement() );
        }
    }
    else {
        qDebug() << "(NepomukOntologyLoader) could not find ontology statements for " << uri;
    }

    return sl;
}

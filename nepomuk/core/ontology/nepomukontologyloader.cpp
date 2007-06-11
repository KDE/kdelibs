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

#include "middleware/registry.h"
#include "middleware/service.h"
#include "middleware/services/rdfrepository.h"

#include <soprano/statement.h>

#include <QtCore/QDebug>


using namespace Nepomuk::RDF;

class Nepomuk::NepomukOntologyLoader::Private
{
public:
    Private()
        : registry( 0 ),
          repository( 0 ) {
    }

    Nepomuk::Middleware::Registry* registry;
    Nepomuk::Services::RDFRepository* repository;
};


Nepomuk::NepomukOntologyLoader::NepomukOntologyLoader()
    : OntologyLoader(),
      d( new Private() )
{
    d->registry = new Nepomuk::Middleware::Registry();
}


Nepomuk::NepomukOntologyLoader::~NepomukOntologyLoader()
{
    delete d->repository;
    delete d->registry;
    delete d;
}


QList<Soprano::Statement> Nepomuk::NepomukOntologyLoader::loadOntology( const QUrl& uri )
{
    if ( !d->repository ) {
        Nepomuk::Middleware::Service* service = d->registry->discoverRDFRepository();
        if ( service ) {
            d->repository = new Nepomuk::Services::RDFRepository( service );
        }
        else {
            qDebug() << "(NepomukOntologyLoader) Could not find RDFRepository service.";
            return QList<Soprano::Statement>();
        }
    }

    if ( d->repository->contains( "main", Soprano::Statement( uri,
                                                              QUrl( RDF::type() ),
                                                              QUrl( NRL::Ontology() ),
                                                              Soprano::Node() ) ) ) {

        // get the complete named graph describing the ontology
        return d->repository->constructSparql( "main",
                                               QString( "construct {?s ?p ?o} "
                                                        "where GRAPH <%1> { ?s ?p ?o. }" )
                                               .arg( uri.toString() ) );
    }
    else {
        qDebug() << "(NepomukOntologyLoader) could not find ontology statements for " << uri;
        return QList<Soprano::Statement>();
    }
}

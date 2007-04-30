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

#include <knepomuk/registry.h>
#include <knepomuk/service.h>
#include <knepomuk/services/rdfrepository.h>

#include <soprano/statement.h>

#include <QtCore/QDebug>


using namespace Nepomuk::RDF;

class Konto::NepomukOntologyLoader::Private
{
public:
    Private()
        : registry( 0 ),
          repository( 0 ) {
    }

    Nepomuk::Backbone::Registry* registry;
    Nepomuk::Services::RDFRepository* repository;
};


Konto::NepomukOntologyLoader::NepomukOntologyLoader()
    : OntologyLoader(),
      d( new Private() )
{
    d->registry = new Nepomuk::Backbone::Registry();
}


Konto::NepomukOntologyLoader::~NepomukOntologyLoader()
{
    delete d->repository;
    delete d->registry;
    delete d;
}


QList<Soprano::Statement> Konto::NepomukOntologyLoader::loadOntology( const QUrl& uri )
{
    if ( !d->repository ) {
        Nepomuk::Backbone::Service* service = d->registry->discoverRDFRepository();
        if ( service ) {
            d->repository = new Nepomuk::Services::RDFRepository( service );
        }
        else {
            qDebug() << "(NepomukOntologyLoader) Could not find RDFRepository service.";
            return QList<Soprano::Statement>();
        }
    }

    if ( d->repository->contains( "main", Soprano::Statement( uri,
                                                              QUrl( rdfsNamespace() + QString( "type" ) ),
                                                              QUrl( nrlNamespace() + QString( "Ontology" ) ),
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

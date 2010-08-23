/* This file is part of the Nepomuk-KDE libraries
    Copyright (c) 2007-2010 Sebastian Trueg <trueg@kde.org>

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
#include "resourcemanager.h"

#include <Soprano/Statement>
#include <Soprano/Model>
#include <Soprano/Node>
#include <Soprano/QueryResultIterator>

#include <QtCore/QDebug>


class Nepomuk::NepomukOntologyLoader::Private
{
public:
};


Nepomuk::NepomukOntologyLoader::NepomukOntologyLoader()
    : OntologyLoader(),
      d( 0 )
{
}


Nepomuk::NepomukOntologyLoader::~NepomukOntologyLoader()
{
}


QList<Soprano::Statement> Nepomuk::NepomukOntologyLoader::loadOntology( const QUrl& uri )
{
    QList<Soprano::Statement> sl;

    // get the complete named graph describing the ontology
    Soprano::QueryResultIterator it
        = ResourceManager::instance()->mainModel()->executeQuery( QString::fromLatin1( "construct {?s ?p ?o} "
                                                                                       "where { GRAPH %1 { ?s ?p ?o } . }" )
                                                                  .arg( Soprano::Node::resourceToN3(uri) ),
                                                                  Soprano::Query::QueryLanguageSparql );
    while ( it.next() ) {
        sl.append( it.currentStatement() );
    }

    return sl;
}

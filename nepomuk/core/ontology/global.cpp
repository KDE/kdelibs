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

#include "global.h"

#include <kdebug.h>
#include <kglobal.h>


class OntologyHelper
{
public:
    OntologyHelper() {
        rdfNS = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";
        rdfType = "http://www.w3.org/1999/02/22-rdf-syntax-ns#type";

        rdfsNS = "http://www.w3.org/2000/01/rdf-schema#";
        rdfsLabel = "http://www.w3.org/2000/01/rdf-schema#label";
        rdfsComment = "http://www.w3.org/2000/01/rdf-schema#comment";

        xmlSchemaNS = "http://www.w3.org/2001/XMLSchema#";
        xmlSchemaInt = "http://www.w3.org/2001/XMLSchema#int";
        xmlSchemaLong = "http://www.w3.org/2001/XMLSchema#long";
        xmlSchemaString = "http://www.w3.org/2001/XMLSchema#string";
        xmlSchemaDouble = "http://www.w3.org/2001/XMLSchema#double";
        xmlSchemaBoolean = "http://www.w3.org/2001/XMLSchema#boolean";

        nrlNS = "http://semanticdesktop.org/ontologies/2006/11/24/nrl#";
        nrlOntology = "http://semanticdesktop.org/ontologies/2006/11/24/nrl#Ontology";

        naoNS = "http://semanticdesktop.org/ontologies/2007/03/31/nao#";
        naoHasIdentifier = "http://semanticdesktop.org/ontologies/2007/03/31/nao#hasIdentifier";

        nfoNS = "http://ont.semanticdesktop.org/2007/03/22/nfo#";
        nfoFile = "http://ont.semanticdesktop.org/2007/03/22/nfo#File";
        nfoFileUrl = "http://ont.semanticdesktop.org/2007/03/22/nfo#fileUrl";
    }

    QString rdfNS;
    QString rdfType;
    QString rdfsNS;
    QString rdfsLabel;
    QString rdfsComment;
    QString xmlSchemaNS;
    QString xmlSchemaInt;
    QString xmlSchemaLong;
    QString xmlSchemaString;
    QString xmlSchemaDouble;
    QString xmlSchemaBoolean;
    QString nrlNS;
    QString nrlOntology;
    QString naoNS;
    QString naoHasIdentifier;
    QString nfoNS;
    QString nfoFile;
    QString nfoFileUrl;
};


K_GLOBAL_STATIC(OntologyHelper, ontoHelper)



QString Nepomuk::RDF::NS()
{
    return ontoHelper->rdfNS;
}


QString Nepomuk::RDF::type()
{
    return ontoHelper->rdfType;
}


QString Nepomuk::RDFS::NS()
{
    return ontoHelper->rdfsNS;
}


QString Nepomuk::RDFS::label()
{
    return ontoHelper->rdfsLabel;
}


QString Nepomuk::RDFS::comment()
{
    return ontoHelper->rdfsComment;
}


QString Nepomuk::XMLSchema::NS()
{
    return ontoHelper->xmlSchemaNS;
}


QString Nepomuk::XMLSchema::xmlInt()
{
    return ontoHelper->xmlSchemaInt;
}


QString Nepomuk::XMLSchema::xmlLong()
{
    return ontoHelper->xmlSchemaLong;
}


QString Nepomuk::XMLSchema::xmlString()
{
    return ontoHelper->xmlSchemaString;
}


QString Nepomuk::XMLSchema::xmlDouble()
{
    return ontoHelper->xmlSchemaDouble;
}


QString Nepomuk::XMLSchema::xmlBoolean()
{
    return ontoHelper->xmlSchemaBoolean;
}


QString Nepomuk::NRL::NS()
{
    return ontoHelper->nrlNS;
}


QString Nepomuk::NRL::Ontology()
{
    return ontoHelper->nrlOntology;
}


QString Nepomuk::NAO::NS()
{
    return ontoHelper->naoNS;
}


QString Nepomuk::NAO::hasIdentifier()
{
    return ontoHelper->naoHasIdentifier;
}


QString Nepomuk::NFO::NS()
{
    return ontoHelper->nfoNS;
}


QString Nepomuk::NFO::File()
{
    return ontoHelper->nfoFile;
}


QString Nepomuk::NFO::fileUrl()
{
    return ontoHelper->nfoFileUrl;
}


QUrl Nepomuk::extractNamespace( const QUrl& url )
{
    QByteArray s = url.toEncoded();
    int pos = s.lastIndexOf( '#' );
    if ( pos == -1 ) {
        pos = s.lastIndexOf( '/' );
    }
    if ( pos == -1 ) {
        kError() << " Failed to extract namespace from " << url << endl;
        return QUrl();
    }
    return QUrl::fromEncoded( s.left( pos+1 ) );
}

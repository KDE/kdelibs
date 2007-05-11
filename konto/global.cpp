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


static const char* RDF_NAMESPACE = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";
static const char* RDFS_NAMESPACE = "http://www.w3.org/2000/01/rdf-schema#";
static const char* XSD_NAMESPACE = "http://www.w3.org/2001/XMLSchema#";
static const char* NRL_NAMESPACE = "http://semanticdesktop.org/ontologies/2006/11/24/nrl#";

QString Konto::rdfNamespace()
{
    return RDF_NAMESPACE;
}


QString Konto::rdfsNamespace()
{
    return RDFS_NAMESPACE;
}


QString Konto::nrlNamespace()
{
    return NRL_NAMESPACE;
}


QString Konto::xsdNamespace()
{
    return XSD_NAMESPACE;
}


QUrl Konto::extractNamespace( const QUrl& url )
{
    QByteArray s = url.toEncoded();
    int pos = s.lastIndexOf( '#' );
    if ( pos == -1 ) {
        pos = s.lastIndexOf( '/' );
    }
    if ( pos == -1 ) {
        kError() << k_funcinfo << " Failed to extract namespace from " << url << endl;
        return QUrl();
    }
    return QUrl::fromEncoded( s.left( pos+1 ) );
}

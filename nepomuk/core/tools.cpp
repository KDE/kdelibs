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

#include "tools.h"
#include "resourcemanager.h"

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

static const char* RDF_NAMESPACE = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";
static const char* RDFS_NAMESPACE = "http://www.w3.org/2000/01/rdf-schema#";
static const char* NRL_NAMESPACE = "http://semanticdesktop.org/ontologies/2006/11/24/nrl#";
static const char* NAO_NAMESPACE = "http://semanticdesktop.org/ontologies/2007/03/31/nao#";

static QString s_customRep;


static QString getLocaleLang()
{
    if( KGlobal::locale() )
        return KGlobal::locale()->language();
    else
        return QLocale::system().name(); // FIXME: does this make sense?
}


void Nepomuk::KMetaData::setDefaultRepository( const QString& s )
{
    s_customRep = s;
}


QString Nepomuk::KMetaData::defaultGraph()
{
    static QString s = "main";
    if( s_customRep.isEmpty() )
        return s;
    else
        return s_customRep;
}


QString Nepomuk::KMetaData::typePredicate()
{
    static QString s = RDF_NAMESPACE + QString( "type" );
    return s;
}


Soprano::Node Nepomuk::KMetaData::valueToRDFNode( const Nepomuk::KMetaData::Variant& v )
{
    return Soprano::Node( Soprano::LiteralValue( v.variant() ) );
}


QList<Soprano::Node> Nepomuk::KMetaData::valuesToRDFNodes( const Nepomuk::KMetaData::Variant& v )
{
    QList<Soprano::Node> nl;

    if( v.isList() ) {
        QStringList vl = v.toStringList();
        for( QStringList::const_iterator it = vl.begin(); it != vl.end(); ++it ) {
            nl.append( Soprano::Node( Soprano::LiteralValue::fromString( *it, ( QVariant::Type )v.simpleType() ) ) );
        }
    }
    else {
        nl.append( valueToRDFNode( v ) );
    }

    return nl;
}


Nepomuk::KMetaData::Variant Nepomuk::KMetaData::RDFLiteralToValue( const Soprano::Node& node )
{
    return node.literal().variant();
}


QString Nepomuk::KMetaData::rdfNamepace()
{
    return QString( RDF_NAMESPACE );
}


QString Nepomuk::KMetaData::rdfsNamespace()
{
    return QString( RDFS_NAMESPACE );
}


QString Nepomuk::KMetaData::nrlNamespace()
{
    return QString( NRL_NAMESPACE );
}


QString Nepomuk::KMetaData::naoNamespace()
{
    return QString( NAO_NAMESPACE );
}

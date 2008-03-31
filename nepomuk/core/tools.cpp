/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2008 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "tools.h"
#include "resourcemanager.h"

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include <QtCore/QLocale>

#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/RDFS>
#include <Soprano/Vocabulary/NRL>
#include <Soprano/Vocabulary/NAO>


static QString getLocaleLang()
{
    if( KGlobal::locale() )
        return KGlobal::locale()->language();
    else
        return QLocale::system().name(); // FIXME: does this make sense?
}


void Nepomuk::setDefaultRepository( const QString& )
{
    // deprecated - do nothing
}


QString Nepomuk::defaultGraph()
{
    static QString s = "main";
    return s;
}


QString Nepomuk::typePredicate()
{
    return Soprano::Vocabulary::RDF::type().toString();
}


Soprano::Node Nepomuk::valueToRDFNode( const Nepomuk::Variant& v )
{
    return Soprano::Node( Soprano::LiteralValue( v.variant() ) );
}


QList<Soprano::Node> Nepomuk::valuesToRDFNodes( const Nepomuk::Variant& v )
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


Nepomuk::Variant Nepomuk::RDFLiteralToValue( const Soprano::Node& node )
{
    return Variant( node.literal().variant() );
}


QString Nepomuk::rdfNamepace()
{
    return Soprano::Vocabulary::RDF::rdfNamespace().toString();
}


QString Nepomuk::rdfsNamespace()
{
    return Soprano::Vocabulary::RDFS::rdfsNamespace().toString();
}


QString Nepomuk::nrlNamespace()
{
    return Soprano::Vocabulary::NRL::nrlNamespace().toString();
}


QString Nepomuk::naoNamespace()
{
    return Soprano::Vocabulary::NAO::naoNamespace().toString();
}

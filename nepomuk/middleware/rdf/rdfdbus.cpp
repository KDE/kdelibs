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

#include "rdfdbus.h"


QDBusArgument& operator<<( QDBusArgument& arg, const Nepomuk::RDF::QueryResultTable& results )
{
    arg.beginStructure();
    arg << results.columns << results.rows;
    arg.endStructure();
    return arg;
}


const QDBusArgument& operator>>( const QDBusArgument& arg, Nepomuk::RDF::QueryResultTable& results )
{
    arg.beginStructure();
    arg >> results.columns >> results.rows;
    arg.endStructure();
    return arg;
}



QDBusArgument& operator<<( QDBusArgument& arg, const Soprano::Node& node )
{
    arg.beginStructure();
    arg << ( int )node.type() << node.toString() << node.language() << node.dataType().toString();
    arg.endStructure();
    return arg;
}


const QDBusArgument& operator>>( const QDBusArgument& arg, Soprano::Node& node )
{
    arg.beginStructure();
    int type;
    QString value, language, dataTypeUri;
    arg >> type >> value >> language >> dataTypeUri;
    if ( type == Soprano::Node::LiteralNode ) {
        node = Soprano::Node( Soprano::LiteralValue::fromString( value, dataTypeUri ), language );
    }
    else {
        node = Soprano::Node( QUrl( value ), ( Soprano::Node::Type )type );
    }
    arg.endStructure();
    return arg;
}


QDBusArgument& operator<<( QDBusArgument& arg, const Soprano::Statement& statement )
{
    arg.beginStructure();
    arg << statement.subject() << statement.predicate() << statement.object() << statement.context();
    arg.endStructure();
    return arg;
}


const QDBusArgument& operator>>( const QDBusArgument& arg, Soprano::Statement& statement )
{
    arg.beginStructure();
    Soprano::Node subject, predicate, object, context;
    arg >> subject >> predicate >> object >> context;
    statement = Soprano::Statement( subject, predicate, object, context );
    arg.endStructure();
    return arg;
}

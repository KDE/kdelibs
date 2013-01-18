/*
   Copyright (c) 2008-2009 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dbusoperators_p.h"

#include <QtDBus/QDBusMetaType>

#include "resource.h"
#include "property.h"

#include <Soprano/Node>
#include <Soprano/BindingSet>


void Nepomuk::Query::registerDBusTypes()
{
    qDBusRegisterMetaType<Nepomuk::Query::Result>();
    qDBusRegisterMetaType<QList<Nepomuk::Query::Result> >();
    qDBusRegisterMetaType<Soprano::Node>();
    qDBusRegisterMetaType<RequestPropertyMapDBus>();
}


QDBusArgument& operator<<( QDBusArgument& arg, const Nepomuk::Query::Result& result )
{
    //
    // Signature: (sda{s(isss)}a{s(isss)}s)
    //

    arg.beginStructure();

    // resource URI and score
    arg << QString::fromLatin1( result.resource().resourceUri().toEncoded() ) << result.score();

    // request properties
    arg.beginMap( QVariant::String, qMetaTypeId<Soprano::Node>() );
    QHash<Nepomuk::Types::Property, Soprano::Node> rp = result.requestProperties();
    for ( QHash<Nepomuk::Types::Property, Soprano::Node>::const_iterator it = rp.constBegin(); it != rp.constEnd(); ++it ) {
        arg.beginMapEntry();
        arg << QString::fromLatin1( it.key().uri().toEncoded() ) << it.value();
        arg.endMapEntry();
    }
    arg.endMap();

    // additional bindings
    arg.beginMap( QVariant::String, qMetaTypeId<Soprano::Node>() );
    const Soprano::BindingSet additionalBindings = result.additionalBindings();
    foreach( const QString& binding, additionalBindings.bindingNames() ) {
        arg.beginMapEntry();
        arg << binding << additionalBindings[binding];
        arg.endMapEntry();
    }
    arg.endMap();

    // full text search excerpt
    arg << result.excerpt();

    arg.endStructure();

    return arg;
}


const QDBusArgument& operator>>( const QDBusArgument& arg, Nepomuk::Query::Result& result )
{
    //
    // Signature: (sda{s(isss)}s)
    //

    arg.beginStructure();
    QString uri;
    double score = 0.0;

    arg >> uri >> score;
    result = Nepomuk::Query::Result( Nepomuk::Resource::fromResourceUri( QUrl::fromEncoded( uri.toLatin1() ) ), score );

    arg.beginMap();
    while ( !arg.atEnd() ) {
        QString rs;
        Soprano::Node node;
        arg.beginMapEntry();
        arg >> rs >> node;
        arg.endMapEntry();
        result.addRequestProperty( QUrl::fromEncoded( rs.toLatin1() ), node );
    }
    arg.endMap();

    Soprano::BindingSet additionalBindings;
    arg.beginMap();
    while ( !arg.atEnd() ) {
        QString binding;
        Soprano::Node node;
        arg.beginMapEntry();
        arg >> binding >> node;
        arg.endMapEntry();
        additionalBindings.insert( binding, node );
    }
    arg.endMap();

    QString excerpt;
    arg >> excerpt;
    result.setExcerpt( excerpt );

    arg.endStructure();

    result.setAdditionalBindings( additionalBindings );

    return arg;
}


QDBusArgument& operator<<( QDBusArgument& arg, const Soprano::Node& node )
{
    arg.beginStructure();
    arg << ( int )node.type();
    if ( node.type() == Soprano::Node::ResourceNode ) {
        arg << QString::fromLatin1( node.uri().toEncoded() );
    }
    else {
        arg << node.toString();
    }
    arg << node.language() << node.dataType().toString();
    arg.endStructure();
    return arg;
}


const QDBusArgument& operator>>( const QDBusArgument& arg, Soprano::Node& node )
{
    //
    // Signature: (isss)
    //
    arg.beginStructure();
    int type;
    QString value, language, dataTypeUri;
    arg >> type >> value >> language >> dataTypeUri;
    if ( type == Soprano::Node::LiteralNode ) {
        if ( dataTypeUri.isEmpty() )
            node = Soprano::Node( Soprano::LiteralValue::createPlainLiteral( value, language ) );
        else
            node = Soprano::Node( Soprano::LiteralValue::fromString( value, QUrl::fromEncoded( dataTypeUri.toLatin1() ) ) );
    }
    else if ( type == Soprano::Node::ResourceNode ) {
        node = Soprano::Node( QUrl::fromEncoded( value.toLatin1() ) );
    }
    else if ( type == Soprano::Node::BlankNode ) {
        node = Soprano::Node( value );
    }
    else {
        node = Soprano::Node();
    }
    arg.endStructure();
    return arg;
}

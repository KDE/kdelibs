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
#include "ontology.h"
#include "resourcemanager.h"

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>


static const QString s_xmlSchemaNs = "http://www.w3.org/2001/XMLSchema#";
static QHash<QString, int> s_xmlSchemaTypes;
static QHash<int, QString> s_variantSchemaTypeHash;
static QString s_customRep;

static void initXmlSchemaTypes()
{
    // FIXME: add proper support for short and byte
    // FIXME: fail if an ontology uses on of the crappy types like integer or negativeInteger
    if( s_xmlSchemaTypes.isEmpty() ) {
        s_xmlSchemaTypes.insert( "int", QVariant::Int );
        s_xmlSchemaTypes.insert( "integer", QVariant::Int );
        s_xmlSchemaTypes.insert( "negativeInteger", QVariant::Int );
        s_xmlSchemaTypes.insert( "decimal", QVariant::Int );
        s_xmlSchemaTypes.insert( "short", QVariant::Int );
        s_xmlSchemaTypes.insert( "long", QVariant::LongLong );
        s_xmlSchemaTypes.insert( "unsignedInt", QVariant::UInt );
        s_xmlSchemaTypes.insert( "unsignedShort", QVariant::UInt );
        s_xmlSchemaTypes.insert( "unsignedLong", QVariant::ULongLong );
        s_xmlSchemaTypes.insert( "boolean", QVariant::Bool );
        s_xmlSchemaTypes.insert( "double", QVariant::Double );
        s_xmlSchemaTypes.insert( "string", QVariant::String );
        s_xmlSchemaTypes.insert( "date", QVariant::Date );
        s_xmlSchemaTypes.insert( "time", QVariant::Time );
        s_xmlSchemaTypes.insert( "dateTime", QVariant::DateTime );
        //    s_xmlSchemaTypes.insert( "", QVariant::Url );
    }

    if( s_variantSchemaTypeHash.isEmpty() ) {
        s_variantSchemaTypeHash.insert( QVariant::Int, "int" );
        s_variantSchemaTypeHash.insert( QVariant::LongLong, "long" );
        s_variantSchemaTypeHash.insert( QVariant::UInt, "unsignedInt" );
        s_variantSchemaTypeHash.insert( QVariant::ULongLong, "unsignedLong" );
        s_variantSchemaTypeHash.insert( QVariant::Bool, "boolean" );
        s_variantSchemaTypeHash.insert( QVariant::Double, "double" );
        s_variantSchemaTypeHash.insert( QVariant::String, "string" );
        s_variantSchemaTypeHash.insert( QVariant::Date, "date" );
        s_variantSchemaTypeHash.insert( QVariant::Time, "time" );
        s_variantSchemaTypeHash.insert( QVariant::DateTime, "dateTime" );
    }
}


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
    static QString s = Ontology::RDF_NAMESPACE + "type";
    return s;
}


Nepomuk::RDF::Node Nepomuk::KMetaData::valueToRDFNode( const Nepomuk::KMetaData::Variant& v )
{
    initXmlSchemaTypes();

    Q_ASSERT( s_variantSchemaTypeHash.contains( v.simpleType() ) );

    Nepomuk::RDF::Node node;
    node.type = Nepomuk::RDF::NodeLiteral;
    //  node.language = getLocaleLang();
    node.dataTypeUri = s_xmlSchemaNs + s_variantSchemaTypeHash[v.simpleType()];
    node.value = v.toString();
    return node;
}


QList<Nepomuk::RDF::Node> Nepomuk::KMetaData::valuesToRDFNodes( const Nepomuk::KMetaData::Variant& v )
{
    initXmlSchemaTypes();

    Q_ASSERT( s_variantSchemaTypeHash.contains( v.simpleType() ) );

    QList<Nepomuk::RDF::Node> nl;

    if( v.isList() ) {
        QStringList vl = v.toStringList();
        for( QStringList::const_iterator it = vl.begin(); it != vl.end(); ++it ) {
            Nepomuk::RDF::Node node;
            node.type = Nepomuk::RDF::NodeLiteral;
            //      node.language = getLocaleLang();
            node.dataTypeUri = s_xmlSchemaNs + s_variantSchemaTypeHash[v.simpleType()];
            node.value = *it;
            nl.append( node );
        }
    }
    else {
        nl.append( valueToRDFNode( v ) );
    }

    return nl;
}


Nepomuk::KMetaData::Variant Nepomuk::KMetaData::RDFLiteralToValue( const Nepomuk::RDF::Node& node )
{
    initXmlSchemaTypes();

    QString dataType = node.dataTypeUri.mid( node.dataTypeUri.lastIndexOf(QRegExp("[\\#\\:]")) + 1 );
    int variantType = QVariant::String;

    if( s_xmlSchemaTypes.contains( dataType ) )
        variantType = s_xmlSchemaTypes[dataType];
    else
        kDebug(300004) << "Unknown literal data type: " << dataType
                       << " (URI: " << node.dataTypeUri << ")" << endl;

    kDebug(300004) << k_funcinfo << " value: " << node.value << " with type " << QMetaType::typeName( variantType ) << endl;
    return Variant::fromString( node.value, variantType );
}




QString Nepomuk::KMetaData::ensureNamespace( const QString& uri )
{
    QString s(uri);
    // very dumb check for a namespace
    // FIXME: improve this
    if( !uri.contains( "://" ) )
        s.prepend( Nepomuk::KMetaData::ResourceManager::instance()->ontology()->defaultNamespace() + '#' );
    return s;
}

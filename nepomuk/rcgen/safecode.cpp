/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING.LIB" for the exact licensing terms.
 */

#include "safecode.h"

#include "property.h"
#include "resourceclass.h"

static const QString s_typeComment =
"    // We always store all Resource types as plain Resource objects.\n"
"    // It does not introduce any overhead (due to the implicit sharing of\n"
"    // the data and has the advantage that we can mix setProperty calls\n"
"    // with the special Resource subclass methods.\n"
"    // More importantly Resource loads the data as Resource objects anyway.\n";

QString SafeCode::propertySetterDeclaration( const Property* property, const ResourceClass* rc, const QString &nameSpace ) const
{
    return QString( "void %1set%2%3%4( const %5& value )" )
        .arg( !nameSpace.isEmpty() ? QString("%1::%2::").arg(nameSpace).arg(rc->name()) : QString() )
        .arg( property->name()[0].toUpper() )
        .arg( property->name().mid(1) )
        .arg( property->isList() ? (property->name().endsWith('s') ? QLatin1String("es") : QLatin1String("s") ) : QString() )
        .arg( property->typeString( false, nameSpace ) );
}

QString SafeCode::propertyGetterDeclaration( const Property* property, const ResourceClass* rc, const QString &nameSpace ) const
{
    return QString( "%1 %2%3%4%5() const" )
        .arg( property->typeString( false, nameSpace ) )
        .arg( !nameSpace.isEmpty() ? QString("%1::%2::").arg(nameSpace).arg(rc->name()) : QString() )
        .arg( property->name()[0].toLower() )
        .arg( property->name().mid(1) )
        .arg( property->isList() ? (property->name().endsWith('s') ? QLatin1String("es") : QLatin1String("s") ) : QString() );
}

QString SafeCode::propertyAdderDeclaration( const Property* property, const ResourceClass* rc, const QString &nameSpace ) const
{
    return QString( "void %1add%2%3( const %4& value )" )
        .arg( !nameSpace.isEmpty() ? QString("%1::%2::").arg(nameSpace).arg(rc->name()) : QString() )
        .arg( property->name()[0].toUpper() )
        .arg( property->name().mid(1) )
        .arg( property->typeString( true, nameSpace ) );
}

QString SafeCode::propertyReversePropertyGetterDeclaration( const Property* property, const ResourceClass* rc, const QString &nameSpace ) const
{
    Q_ASSERT( rc );
    Q_ASSERT( property->domain() );
    return QString( "%1 %2%3%4Of() const" )
        .arg( QString("QList<") + property->domain(true)->name( nameSpace ) + QString(">") )
        .arg( !nameSpace.isEmpty() ? QString("%1::%2::").arg(nameSpace).arg(rc->name()) : QString() )
        .arg( property->name()[0].toLower() )
        .arg( property->name().mid(1) );
}

QString SafeCode::resourceAllResourcesDeclaration( const ResourceClass* rc, const QString &nameSpace ) const
{
    return QString( "QList<%1%2> %3all%2s()" )
        .arg( !nameSpace.isEmpty() ? nameSpace + "::" : QString() )
        .arg( rc->name() )
        .arg( !nameSpace.isEmpty() ? QString("%1::%2::").arg(nameSpace).arg( rc->name() ) : QString() );
}

QString SafeCode::resourcePseudoInheritanceDeclaration( const ResourceClass* baseRc, const ResourceClass* rc, const QString &nameSpace ) const
{
    return QString( "%1 %2to%3() const" )
        .arg( rc->name( nameSpace ) )
        .arg( !nameSpace.isEmpty() ? baseRc->name( "Nepomuk" ) + "::" : QString() )
        .arg( rc->name() );
}

QString SafeCode::propertySetterDefinition( const Property* property, const ResourceClass* rc ) const
{
    QString s = propertySetterDeclaration( property, rc, "Nepomuk" ) + '\n';

    if( property->hasSimpleType() || property->typeString( true ) == "Resource" || !property->isList() ) {
        s += QString("{\n"
                     "    setProperty( QUrl::fromEncoded(\"%1\"), Variant( value ) );\n"
                     "}\n" )
             .arg( property->uri().toString() );
    }
    else if( property->isList() ) {
        s += QString("{\n"
                     "%1"
                     "    QList<Resource> l;\n"
                     "    for( %2::const_iterator it = value.constBegin();\n"
                     "         it != value.constEnd(); ++it ) {\n"
                     "        l.append( Resource( (*it) ) );\n"
                     "    }\n"
                     "    setProperty( QUrl::fromEncoded(\"%3\"), Variant( l ) );\n"
                     "}\n" )
             .arg( s_typeComment )
             .arg( property->typeString() )
             .arg( property->uri().toString() );
    }
    else {
        s += QString("{\n"
                     "%1"
                     "    setProperty( QUrl::fromEncoded(\"%2\"), Variant( Resource( value ) ) );\n"
                     "}\n" )
             .arg( s_typeComment )
             .arg( property->uri().toString() );
    }

    return s;
}

QString SafeCode::propertyGetterDefinition( const Property* property, const ResourceClass* rc ) const
{
    QString s = propertyGetterDeclaration( property, rc, "Nepomuk" ) + '\n';

    if( property->hasSimpleType() ) {
        s += QString( "{\n"
                      "    return ( property( QUrl::fromEncoded(\"%1\") ).%2;\n"
                      "}\n" )
             .arg( property->uri().toString() )
             .arg( property->literalTypeConversionMethod() );
    }
    else if( property->isList() ) {
        s += QString("{\n"
                     "%1"
                     "    return convertResourceList<%3>( property( QUrl::fromEncoded(\"%2\") ).toResourceList() );\n"
                     "}\n" )
             .arg( s_typeComment )
             .arg( property->uri().toString() )
             .arg( property->typeString( true ) );
    }
    else {
        s += QString("{\n"
                     "%1"
                     "    return %2( property( QUrl::fromEncoded(\"%3\") ).toResource().resourceUri() );\n"
                     "}\n" )
             .arg( s_typeComment )
             .arg( property->typeString( true ) )
             .arg( property->uri().toString() );
    }

    return s;
}

QString SafeCode::propertyAdderDefinition( const Property* property, const ResourceClass* rc ) const
{
    QString s = propertyAdderDeclaration( property, rc, "Nepomuk" ) + '\n';

    if( property->hasSimpleType() ) {
        s += QString( "{\n"
                      "    Variant v = property( QUrl::fromEncoded(\"%1\") );\n"
                      "    v.append( value );\n"
                      "    setProperty( QUrl::fromEncoded(\"%1\"), v );\n"
                      "}\n" )
             .arg( property->uri().toString() );
    }
    else {
        s += QString( "{\n"
                      "%1"
                      "    Variant v = property( QUrl::fromEncoded(\"%2\") );\n"
                      "    v.append( Resource( value ) );\n"
                      "    setProperty( QUrl::fromEncoded(\"%2\"), v );\n"
                      "}\n" )
             .arg( s_typeComment )
             .arg( property->uri().toString() );
    }

    return s;
}

QString SafeCode::propertyReversePropertyGetterDefinition( const Property* property, const ResourceClass* rc ) const
{
    QString s = propertyReversePropertyGetterDeclaration( property, rc, "Nepomuk" ) + '\n';

    s += QString( "{\n"
                  "    return convertResourceList<%2>( manager()->allResourcesWithProperty( QUrl::fromEncoded(\"%1\"), *this ) );\n"
                  "}\n" )
         .arg( property->uri().toString() )
         .arg( property->domain(true)->name() );

    return s;
}

QString SafeCode::resourceAllResourcesDefinition( const ResourceClass* rc ) const
{
    return QString( "%1\n"
                    "{\n"
                    "    return Nepomuk::convertResourceList<%3>( ResourceManager::instance()->allResourcesOfType( QUrl::fromEncoded(\"%2\") ) );\n"
                    "}\n" )
        .arg( resourceAllResourcesDeclaration( rc, "Nepomuk" ) )
        .arg( rc->uri().toString() )
        .arg( rc->name() );
}

QString SafeCode::resourcePseudoInheritanceDefinition( const ResourceClass* baseRc, const ResourceClass* rc ) const
{
    return QString( "%1\n"
                    "{\n"
                    "    return %2( *this );\n"
                    "}\n" )
        .arg( resourcePseudoInheritanceDeclaration( baseRc, rc, "Nepomuk" ) )
        .arg( rc->name( "Nepomuk" ) );
}

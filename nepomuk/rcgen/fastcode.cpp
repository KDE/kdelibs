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

#include "fastcode.h"

#include "property.h"
#include "resourceclass.h"

QString FastCode::propertySetterDeclaration( const Property* property, const ResourceClass* rc, const QString &nameSpace ) const
{
    return QString( "void %1set%2%3%4( const %5& value )" )
        .arg( !nameSpace.isEmpty() ? QString("%1::%2::").arg(nameSpace).arg(rc->name()) : QString() )
        .arg( property->name()[0].toUpper() )
        .arg( property->name().mid(1) )
        .arg( property->isList() ? (property->name().endsWith('s') ? QLatin1String("es") : QLatin1String("s") ) : QString() )
        .arg( property->typeString( false, nameSpace ) );
}

QString FastCode::propertyGetterDeclaration( const Property* property, const ResourceClass* rc, const QString &nameSpace ) const
{
    Q_UNUSED( property );
    Q_UNUSED( rc );
    Q_UNUSED( nameSpace );
    return QString();
}

QString FastCode::propertyAdderDeclaration( const Property* property, const ResourceClass* rc, const QString &nameSpace ) const
{
    return QString( "void %1add%2%3( const %4& value )" )
        .arg( !nameSpace.isEmpty() ? QString("%1::%2::").arg(nameSpace).arg(rc->name()) : QString() )
        .arg( property->name()[0].toUpper() )
        .arg( property->name().mid(1) )
        .arg( property->typeString( true, nameSpace ) );
}

QString FastCode::propertyReversePropertyGetterDeclaration( const Property* property, const ResourceClass* rc, const QString &nameSpace ) const
{
    Q_UNUSED( property );
    Q_UNUSED( rc );
    Q_UNUSED( nameSpace );
    return QString();
}

QString FastCode::resourceAllResourcesDeclaration( const ResourceClass* rc, const QString &nameSpace ) const
{
    Q_UNUSED( rc );
    Q_UNUSED( nameSpace );
    return QString();
}

QString FastCode::resourcePseudoInheritanceDeclaration( const ResourceClass* baseRc, const ResourceClass* rc, const QString &nameSpace ) const
{
    return QString( "%1 %2to%3() const" )
        .arg( rc->name( nameSpace ) )
        .arg( !nameSpace.isEmpty() ? baseRc->name( "NepomukFast" ) + "::" : QString() )
        .arg( rc->name() );
}

QString FastCode::propertySetterDefinition( const Property* property, const ResourceClass* rc ) const
{
    QString s = propertySetterDeclaration( property, rc, "NepomukFast" ) + '\n';

    s += QLatin1String("{\n");
    if ( !property->isList() ) {
        if ( property->hasSimpleType() ) {
            s += QString::fromLatin1("    addProperty( QUrl::fromEncoded(\"%1\"), Soprano::LiteralValue( value ) );\n").arg( property->uri().toString() );
        } else {
            s += QString::fromLatin1("    addProperty( QUrl::fromEncoded(\"%1\"), value.uri() );\n").arg( property->uri().toString() );
        }
    } else {
        if ( property->hasSimpleType() ) {
            s += QString::fromLatin1("    for( %1::const_iterator it = value.constBegin();\n"
                                     "         it != value.constEnd(); ++it ) {\n"
                                     "       addProperty( QUrl::fromEncoded(\"%2\"), Soprano::LiteralValue( *it ) );\n"
                                     "    }\n").arg( property->typeString() ).arg( property->uri().toString() );
        } else {
            s += QString::fromLatin1("    for( %1::const_iterator it = value.constBegin();\n"
                                     "         it != value.constEnd(); ++it ) {\n"
                                     "       addProperty( QUrl::fromEncoded(\"%2\"), (*it).uri() );\n"
                                     "    }\n").arg( property->typeString() ).arg( property->uri().toString() );
        }
    }
    s += QLatin1String("}\n");

    return s;
}

QString FastCode::propertyGetterDefinition( const Property* property, const ResourceClass* rc ) const
{
    Q_UNUSED( property );
    Q_UNUSED( rc );
    return QString();
}

QString FastCode::propertyAdderDefinition( const Property* property, const ResourceClass* rc ) const
{
    QString s = propertyAdderDeclaration( property, rc, "NepomukFast" ) + '\n';

    s += QLatin1String("{\n");
    if ( property->hasSimpleType() ) {
        s += QString::fromLatin1("    addProperty( QUrl::fromEncoded(\"%1\"), Soprano::LiteralValue( value ) );\n").arg( property->uri().toString() );
    } else {
        s += QString::fromLatin1("    addProperty( QUrl::fromEncoded(\"%1\"), value.uri() );\n").arg( property->uri().toString() );
    }
    s += QLatin1String("}\n");

    return s;
}

QString FastCode::propertyReversePropertyGetterDefinition( const Property* property, const ResourceClass* rc ) const
{
    Q_UNUSED( property );
    Q_UNUSED( rc );
    return QString();
}

QString FastCode::resourceAllResourcesDefinition( const ResourceClass* rc ) const
{
    Q_UNUSED( rc );
    return QString();
}

QString FastCode::resourcePseudoInheritanceDefinition( const ResourceClass* baseRc, const ResourceClass* rc ) const
{
    return QString( "%1\n"
                    "{\n"
                    "    return %2( uri(), graphUri() );\n"
                    "}\n" )
        .arg( resourcePseudoInheritanceDeclaration( baseRc, rc, "NepomukFast" ) )
        .arg( rc->name( "NepomukFast" ) );
}

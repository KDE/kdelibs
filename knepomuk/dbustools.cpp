/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING.LIB" for the exact licensing terms.
 */

#include "dbustools.h"

QString Nepomuk::Backbone::DBus::dbusServiceFromUrl( const QString& uri )
{
    QString dbusService;
    int pos = uri.indexOf( ':' );
    if( pos > 0 ) {
        ++pos;
        // skip all following slashes
        while( pos < (int)uri.length() && uri[pos] == '/' )
            ++pos;

        // uri without the protocol
        dbusService = uri.mid( pos );
    }
    else
        dbusService = uri;

    // invert the order of the domain name
    QStringList tokens = dbusService.section( '/', 0, 0 ).split( '.' );
    dbusService = dbusService.section( '/', 1 );
    for( int i = 0; i < tokens.count(); ++i ) {
        dbusService.prepend( tokens[i] + '.' );
    }

    // remove all slashes
    dbusService.replace( '/', '.' );

    return dbusService;
}


QString Nepomuk::Backbone::DBus::dbusObjectFromType( const QString& uri )
{
    return '/' + dbusInterfaceFromType( uri ).replace( '.', '/' );
}


QString Nepomuk::Backbone::DBus::dbusInterfaceFromType( const QString& uri )
{
    return dbusServiceFromUrl( uri );
}


QDBusArgument& operator<<( QDBusArgument& arg, const Nepomuk::Backbone::ServiceDesc& desc )
{
    arg.beginStructure();
    arg << desc.name << desc.url << desc.type;
    arg.endStructure();
    return arg;
}


const QDBusArgument& operator>>( const QDBusArgument& arg, Nepomuk::Backbone::ServiceDesc& desc )
{
    arg.beginStructure();
    arg >> desc.name >> desc.url >> desc.type;
    arg.endStructure();
    return arg;
}

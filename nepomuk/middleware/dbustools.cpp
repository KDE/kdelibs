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

#include "dbustools.h"

QString Nepomuk::Middleware::DBus::dbusServiceFromUrl( const QString& uri )
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


QString Nepomuk::Middleware::DBus::dbusObjectFromType( const QString& uri )
{
    return '/' + dbusInterfaceFromType( uri ).replace( '.', '/' );
}


QString Nepomuk::Middleware::DBus::dbusInterfaceFromType( const QString& uri )
{
    return dbusServiceFromUrl( uri );
}


QDBusArgument& operator<<( QDBusArgument& arg, const Nepomuk::Middleware::ServiceDesc& desc )
{
    arg.beginStructure();
    arg << desc.name << desc.url << desc.type;
    arg.endStructure();
    return arg;
}


const QDBusArgument& operator>>( const QDBusArgument& arg, Nepomuk::Middleware::ServiceDesc& desc )
{
    arg.beginStructure();
    arg >> desc.name >> desc.url >> desc.type;
    arg.endStructure();
    return arg;
}

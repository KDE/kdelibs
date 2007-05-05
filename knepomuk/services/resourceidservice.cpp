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

#include "resourceidservice.h"
#include <knepomuk/service.h>
#include <knepomuk/message.h>
#include <knepomuk/result.h>


Nepomuk::Services::ResourceIdService::ResourceIdService( Backbone::Service* s )
    : Backbone::ServiceWrapper( s )
{
}


QString Nepomuk::Services::ResourceIdService::toUniqueUrl( const QString& url )
{
    Backbone::Message m( service()->url(), "toUniqueUrl" );
    m.addArgument( url );

    Backbone::Result reply = service()->methodCall( m );
    if( !reply.status() )
        return reply.value().value<QString>();
    else
        return QString();
}


QString Nepomuk::Services::ResourceIdService::fromUniqueUrl( const QString& url )
{
    Backbone::Message m( service()->url(), "fromUniqueUrl" );
    m.addArgument( url );

    Backbone::Result reply = service()->methodCall( m );
    if( !reply.status() )
        return reply.value().value<QString>();
    else
        return QString();
}

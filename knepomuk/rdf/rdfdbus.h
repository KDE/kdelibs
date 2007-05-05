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

#ifndef _NEPOMUK_BACKBONE_SERVICES_RDF_DBUS_H_
#define _NEPOMUK_BACKBONE_SERVICES_RDF_DBUS_H_

#include <knepomuk/rdf/queryresulttable.h>
#include <knepomuk/knepomuk_export.h>

#include <soprano/node.h>
#include <soprano/statement.h>

#include <QtDBus/QDBusArgument>
#include <QtCore/QList>


KNEPOMUK_EXPORT QDBusArgument& operator<<( QDBusArgument& arg, const Nepomuk::RDF::QueryResultTable& );
KNEPOMUK_EXPORT const QDBusArgument& operator>>( const QDBusArgument& arg, Nepomuk::RDF::QueryResultTable& );

KNEPOMUK_EXPORT QDBusArgument& operator<<( QDBusArgument& arg, const Soprano::Node& );
KNEPOMUK_EXPORT const QDBusArgument& operator>>( const QDBusArgument& arg, Soprano::Node& );

KNEPOMUK_EXPORT QDBusArgument& operator<<( QDBusArgument& arg, const Soprano::Statement& );
KNEPOMUK_EXPORT const QDBusArgument& operator>>( const QDBusArgument& arg, Soprano::Statement& );


Q_DECLARE_METATYPE(Soprano::Node)
Q_DECLARE_METATYPE(Soprano::Statement)
Q_DECLARE_METATYPE(QList<Soprano::Node>)
Q_DECLARE_METATYPE(QList<Soprano::Statement>)

#endif

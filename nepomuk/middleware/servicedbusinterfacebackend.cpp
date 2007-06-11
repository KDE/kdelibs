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

#include "servicedbusinterfacebackend.h"
#include "message.h"
#include "result.h"
#include "dbustools.h"
#include "rdf/queryresulttable.h"
#include "rdf/rdfdbus.h"

#include <soprano/statement.h>

#include <QtDBus>

#include <kdebug.h>


Nepomuk::Middleware::DBus::ServiceBackend::ServiceBackend( const QString& url,
							 const QString& type,
							 QObject* parent )
    : QDBusInterface( dbusServiceFromUrl(url),
                      dbusObjectFromType(type),
                      dbusInterfaceFromType(type),
                      QDBusConnection::sessionBus(),
                      parent )
{
}


Nepomuk::Middleware::DBus::ServiceBackend::~ServiceBackend()
{
}


bool Nepomuk::Middleware::DBus::ServiceBackend::isAvailable() const
{
    // FIXME: I don't think this is sufficient
    return isValid();
}


Nepomuk::Middleware::Result Nepomuk::Middleware::DBus::ServiceBackend::methodCall( const Nepomuk::Middleware::Message& message )
{
     QDBusMessage msg = QDBusMessage::createMethodCall( service(), path(), interface(), message.method() );
     msg.setArguments( message.arguments() );
     QDBusMessage dbusReply = QDBusConnection::sessionBus().call( msg, QDBus::Block, 10*60*1000 ); // timeout of 10 minutes for testing

//    QDBusMessage dbusReply = callWithArgumentList( QDBus::Block,
//                                                   message.method(),
//                                                   message.arguments() );

    if( dbusReply.type() == QDBusMessage::ErrorMessage ) {
        if ( dbusReply.arguments().count() > 0 ) {
            kDebug(300001) << "(DBus::ServiceBackend) got error reply: "
                           << dbusReply.errorName() << " - " << dbusReply.arguments().first().toString() << endl;
            return Result::createErrorResult( -1, dbusReply.errorName(), dbusReply.arguments().first().toString() );
        }
        else {
            kDebug(300001) << "(DBus::ServiceBackend) got error reply: " << dbusReply.errorName() << endl;
            return Result::createErrorResult( -1, dbusReply.errorName() );
        }
    }
    else if( dbusReply.type() != QDBusMessage::ReplyMessage ) {
        kDebug(300001) << "(DBus::ServiceBackend) got invalid reply while calling method "
                       << path() + '/' + interface() + '.' + message.method() << message.arguments()
                       << "in service" << service() << ": " << dbusReply.errorName() << endl;
        return Result::createErrorResult( -1, dbusReply.errorName() );
    }

    // FIXME: we silently assume that the method has only a single return value

    if ( dbusReply.arguments().count() ) {
        //
        // Let QtDBus handle the simple types
        //
        if( dbusReply.arguments()[0].userType() < (int)QVariant::UserType ) {
            return Result::createSimpleResult( dbusReply.arguments()[0] );
        }

        //
        // Check for the non-simple types
        //
        else {
            QDBusArgument arg = dbusReply.arguments()[0].value<QDBusArgument>();

            // FIXME: analyze the signature: [1. let QtDBus extract the simple types]
            //                               2. check for the RDF cursor types

            //
            // Check for our RDF cursor types
            //

            // FIXME: store the cursor type signatures somewhere else
            if( dbusReply.signature() == "a((isss)(isss)(isss)(isss))" ) {
                QList<Soprano::Statement> sl;
                arg >> sl;
                return Result::createSimpleResult( qVariantFromValue<QList<Soprano::Statement> >( sl ) );
            }
            else if( dbusReply.signature() == "(asaa(isss))" ) {
                RDF::QueryResultTable qr;
                arg >> qr;
                return Result::createSimpleResult( qVariantFromValue<RDF::QueryResultTable>( qr ) );
            }

            //
            // Check for lists of lists of maps of ...
            //
            else {
                kDebug(300001) << "(DBus::ServiceBackend) signature: " << dbusReply.signature() << endl;
                //                               3. check for lists of lists of maps and so on
                //                               4. fail all other types
                return Result::createErrorResult( -1, "org.semanticdesktop.nepomuk.error.UnknownType",
                                                  QString("The D-Bus signature %1 is not supported.").arg(dbusReply.signature()) );
            }
        }
    }
    else {
        return Result();
    }
}

#include "servicedbusinterfacebackend.moc"

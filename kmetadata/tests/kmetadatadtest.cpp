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

#include "kmetadatadtest.h"
#include <kmetadata/kmetadata.h>

#include <QtDBus>


using namespace Nepomuk::KMetaData;


void KMetaDatadTest::initTestCase()
{
    ResourceManager::instance()->setAutoSync( false );
}


void KMetaDatadTest::testDBusInterface()
{
    QDBusMessage getPropMsg = QDBusMessage::createMethodCall( "org.semanticdesktop.nepomuk.kmetadata",
                                                              "/org/semanticdesktop/nepomuk/kmetadata",
                                                              "org.semanticdesktop.nepomuk.KMetaData",
                                                              "getProperty" );
    QDBusMessage setPropMsg = QDBusMessage::createMethodCall( "org.semanticdesktop.nepomuk.kmetadata",
                                                              "/org/semanticdesktop/nepomuk/kmetadata",
                                                              "org.semanticdesktop.nepomuk.KMetaData",
                                                              "setProperty" );
    QDBusMessage removePropMsg = QDBusMessage::createMethodCall( "org.semanticdesktop.nepomuk.kmetadata",
                                                                 "/org/semanticdesktop/nepomuk/kmetadata",
                                                                 "org.semanticdesktop.nepomuk.KMetaData",
                                                                 "removeProperty" );
    QDBusMessage removeResMsg = QDBusMessage::createMethodCall( "org.semanticdesktop.nepomuk.kmetadata",
                                                                "/org/semanticdesktop/nepomuk/kmetadata",
                                                                "org.semanticdesktop.nepomuk.KMetaData",
                                                                "removeResource" );

    QString tmpAnnot = "This is the default Unix temp dir.";
    QList<QVariant> args;

    // set property
    //
    args.clear();
    args << QString("/tmp") << QString("hasComment") << tmpAnnot;
    setPropMsg.setArguments( args );
    QVERIFY( QDBusConnection::sessionBus().call( setPropMsg ).type() != QDBusMessage::ErrorMessage );

    // get property
    //
    args.clear();
    args << QString("/tmp") << QString("hasComment");
    getPropMsg.setArguments( args );
    QDBusReply<QString> r = QDBusConnection::sessionBus().call( getPropMsg );

    QCOMPARE( r.value(), tmpAnnot );

    // just to make sure
    Resource res( "/tmp" );
    QCOMPARE( res.getComment(), tmpAnnot );

    // remove property
    //
    args.clear();
    args << QString("/tmp") << QString("hasComment");
    removePropMsg.setArguments( args );
    QVERIFY( QDBusConnection::sessionBus().call( removePropMsg ).type() != QDBusMessage::ErrorMessage );

    // get property
    //
    args.clear();
    args << QString("/tmp") << QString("hasComment");
    getPropMsg.setArguments( args );
    r = QDBusConnection::sessionBus().call( getPropMsg );

    QVERIFY( r.value().isEmpty() );

    QVERIFY( res.exists() );

    // remove resource
    //
    args.clear();
    args << QString("/tmp");
    removeResMsg.setArguments( args );
    QVERIFY( QDBusConnection::sessionBus().call( removeResMsg ).type() != QDBusMessage::ErrorMessage );

    QVERIFY( !res.exists() );
}


QTEST_MAIN(KMetaDatadTest)

#include "kmetadatadtest.moc"

/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING.LIB" for the exact licensing terms.
 */

#include "varianttest.h"
#include "../kmetadata/kmetadata.h"

#include <qtest_kde.h>

using namespace Nepomuk::KMetaData;

Q_DECLARE_METATYPE(Nepomuk::KMetaData::Variant)

void VariantTest::initTestCase()
{
    QVERIFY( ResourceManager::instance()->init() == 0 );
}


void VariantTest::testToString_data()
{
    // test comparison
    QTest::addColumn<Variant>( "a" );
    QTest::addColumn<Variant>( "b" );

    QTest::newRow("string-string") << Variant( "test" ) << Variant( "test" );
    QTest::newRow("string-int") << Variant( "42" ) << Variant( 42 );
    QTest::newRow("string-false") << Variant( "false" ) << Variant( false );
    QTest::newRow("string-true") << Variant( "true" ) << Variant( true );
    QTest::newRow("string-double") << Variant( "42.2" ) << Variant( 42.2 );
//  QTest::newRow("string-resource") << Variant( Resource("test") ) << Variant( "test" );

    QList<int> intL;
    QList<bool> boolL;
    QList<double> doubleL;
    QList<Resource> resL;

    intL << 42 << 2 << -18 << 100000;
    boolL << true << true << false << true;
    doubleL << 42.5 << 2.5 << -18.5 << 1000.5;
    resL << Resource( "/home/test/wurst" ) << Resource( "hello" ) << Resource( "/tmp" );
    QStringList uriL;
    uriL << "/home/test/wurst" << "hello" << "/tmp";

    QTest::newRow("int-list") << Variant( intL ) << Variant( QString("42,2,-18,100000").split(",") );
    QTest::newRow("bool-list") << Variant( boolL ) << Variant( QString("true,true,false,true").split(",") );
    QTest::newRow("double-list") << Variant( doubleL ) << Variant( QString("42.5,2.5,-18.5,1000.5").split(",") );
//  QTest::newRow("res-list") << Variant( resL ) << Variant( uriL );
}


void VariantTest::testToString()
{
    QFETCH(Variant, a);
    QFETCH(Variant, b);
    QCOMPARE( a.toString(), b.toString() );
    QCOMPARE( a.toStringList(), b.toStringList() );
}


QTEST_KDEMAIN(VariantTest, NoGUI)

#include "varianttest.moc"

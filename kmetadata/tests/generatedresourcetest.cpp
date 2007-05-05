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

#include "generatedresourcetest.h"
#include <kmetadata/kmetadata.h>
#include <kmetadata/file.h>

using namespace Nepomuk::KMetaData;


void GeneratedResourceTest::testUriHandling()
{
    QString s = "test";

    File file( "/tmp" );
    file.setComment( s );

    // FIXME: use the ontology parser (which is not done yet)
//   QCOMPARE( file.getProperty( "nkde:hasAnnotation" ).toString(), s );

//   Resource res( "/tmp" );

//   QCOMPARE( file.getAnnotation(), res.getProperty( "nkde:hasAnnotation" ).toString() );
//   QCOMPARE( res.getProperty( "nkde:hasAnnotation" ).toString(), s );
}


QTEST_MAIN(GeneratedResourceTest)

#include "generatedresourcetest.moc"

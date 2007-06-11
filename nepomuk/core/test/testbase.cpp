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

#include "testbase.h"
#include <kmetadata/kmetadata.h>
#include <knepomuk/knepomuk.h>
#include <knepomuk/services/rdfrepository.h>

using namespace Nepomuk;
using namespace Nepomuk::KMetaData;
using namespace Soprano;

void TestBase::initTestCase()
{
    qDebug() << "Initializing KMetaData TestBase..." << endl;

    setDefaultRepository( "testRep" );
    QVERIFY( ResourceManager::instance()->init() == 0 );
}

void TestBase::init()
{
    // clean out all statements
    Nepomuk::Services::RDFRepository rr( ResourceManager::instance()->serviceRegistry()->discoverRDFRepository() );
    rr.removeAllStatements( "testRep", Statement() );
}


void TestBase::cleanup()
{
}

#include "testbase.moc"

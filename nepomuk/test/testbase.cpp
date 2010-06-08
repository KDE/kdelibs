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

#include "resource.h"
#include "resourcemanager.h"

#include <Soprano/Soprano>

#include <ktempdir.h>

void TestBase::initTestCase()
{
    m_tmpDir = new KTempDir();
    const Soprano::Backend* backend = Soprano::PluginManager::instance()->discoverBackendByName("virtuoso");
    m_model = backend->createModel( Soprano::BackendSettings() << Soprano::BackendSetting(Soprano::BackendOptionStorageDir, m_tmpDir->name() ));
    Nepomuk::ResourceManager::instance()->setOverrideMainModel( m_model );
}


void TestBase::cleanupTestCase()
{
    Nepomuk::ResourceManager::instance()->setOverrideMainModel( 0 );
    delete m_model;
    delete m_tmpDir;
}


void TestBase::init()
{
    m_model->removeAllStatements();
}


void TestBase::cleanup()
{
}

#include "testbase.moc"

/* This file is part of the KDE libraries
    Copyright (c) 2005 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QtTest/qttest_kde.h>
#include "klibloadertest.h"
#include "klibloadertest.moc"

QTTEST_KDEMAIN( KLibLoaderTest, NoGUI )

#include <klibloader.h>
#include <kstandarddirs.h>
#include <qdir.h>
#include <kdebug.h>

void KLibLoaderTest::testNonWorking()
{
    int error = 0;
    QObject* obj = KLibLoader::createInstance<QObject>( "idontexist", this, 0, QStringList(), &error );
    COMPARE( obj, (QObject*)0 );
    COMPARE( error, (int)KLibLoader::ErrNoLibrary );
}

// We need a module to dlopen, which uses a standard factory (e.g. not an ioslave)
static const char* s_module = "libklibloadertestmodule";

void KLibLoaderTest::testWorking()
{
    int error = 0;
    KGlobal::dirs()->addResourceDir( "module", QDir::currentPath() );
    QObject* obj = KLibLoader::createInstance<QObject>( s_module, 0, 0, QStringList(), &error );
    if ( error )
        kdWarning() << "error=" << error << " lastErrorMessage=" << KLibLoader::self()->lastErrorMessage() << endl;
    VERIFY( obj != 0 );
}

void KLibLoaderTest::testWrongClass()
{
    KGlobal::dirs()->addResourceDir( "module", QDir::currentPath() );
    int error = 0;

    KLibLoaderTest* obj = KLibLoader::createInstance<KLibLoaderTest>( s_module, 0, 0, QStringList(), &error );
    COMPARE( obj, (KLibLoaderTest*)0 );
    COMPARE( error, (int)KLibLoader::ErrNoComponent );
}

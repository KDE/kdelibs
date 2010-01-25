/* This file is part of the KDE libraries
    Copyright (c) 2005-2006 David Faure <faure@kde.org>

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

#include <qtest_kde.h>
#include "klibloadertest.h"
#include "klibloadertest.moc"

QTEST_KDEMAIN_CORE( KLibLoaderTest )

#include <klibloader.h>
#include <kstandarddirs.h>
#include <QtCore/QDir>
#include <kdebug.h>

void KLibLoaderTest::initTestCase()
{
    const QString libdir = QDir::currentPath() + "/../../lib";
    KGlobal::dirs()->addResourceDir( "module", libdir );
    //qDebug( "initTestCase: added %s to 'module' resource", qPrintable(libdir) );
}

void KLibLoaderTest::testNonWorking()
{
    int error = 0;
    QObject* obj = KLibLoader::createInstance<QObject>( "idontexist", this, QStringList(), &error );
    QCOMPARE( obj, (QObject*)0 );
    QCOMPARE( error, (int)KLibLoader::ErrNoLibrary );
    QString errorString = KLibLoader::errorString( error );
    kDebug() << errorString;
    QVERIFY( !errorString.isEmpty() );
}

// We need a module to dlopen, which uses a standard factory (e.g. not an ioslave)
static const char s_kgenericFactoryModule[] = "klibloadertestmodule";

void KLibLoaderTest::testFindLibrary()
{
    const QString library = KLibLoader::findLibrary( s_kgenericFactoryModule );
    QVERIFY( !library.isEmpty() );
    const QString libraryPath = QFileInfo( library ).canonicalFilePath();
#if defined(Q_OS_WIN) || defined(Q_OS_CYGWIN)
    const QString expectedPath = QFileInfo( QDir::currentPath() + "/../../lib/" + s_kgenericFactoryModule + ".dll" ).canonicalFilePath();
#else
    const QString expectedPath = QFileInfo( QDir::currentPath() + "/../../lib/" + s_kgenericFactoryModule + ".so" ).canonicalFilePath();
#endif
    QCOMPARE( library, expectedPath );
}

// old loader, old plugin
void KLibLoaderTest::testWorking_KLibLoader_KGenericFactory()
{
    int error = 0;
    QObject* obj = KLibLoader::createInstance<QObject>( s_kgenericFactoryModule, 0, QStringList(), &error );
    if ( error )
        kWarning() << "error=" << error << " lastErrorMessage=" << KLibLoader::self()->lastErrorMessage();
    QVERIFY( obj != 0 );
    delete obj;
}

// old loader, old plugin
void KLibLoaderTest::testWrongClass_KLibLoader_KGenericFactory()
{
    int error = 0;

    KLibLoaderTest* obj = KLibLoader::createInstance<KLibLoaderTest>( s_kgenericFactoryModule, 0, QStringList(), &error );
    QCOMPARE( obj, (KLibLoaderTest*)0 );
    QCOMPARE( error, (int)KLibLoader::ErrNoComponent );
    QString errorString = KLibLoader::errorString( error );
    kDebug() << errorString;
    QVERIFY( !errorString.isEmpty() );
}

static const char s_kpluginFactoryModule[] = "klibloadertestmodule4";

// old loader, new plugin
void KLibLoaderTest::testWorking_KLibLoader_KPluginFactory()
{
    int error = 0;
    QObject* obj = KLibLoader::createInstance<QObject>( s_kpluginFactoryModule, 0, QStringList(), &error );
    if ( error )
        kWarning() << "error=" << error << " lastErrorMessage=" << KLibLoader::self()->lastErrorMessage();
    QVERIFY( obj != 0 );
    delete obj;
}

// new loader, old plugin
void KLibLoaderTest::testWorking_KPluginLoader_KGenericFactory()
{
    KPluginLoader loader(s_kgenericFactoryModule);
    KPluginFactory* factory = loader.factory();
    if (!factory) {
        kWarning() << "error=" << loader.errorString();
        QVERIFY(factory);
    } else {
        QObject* obj = factory->create<QObject>();
        if (!obj) {
            kWarning() << "Error creating object";
        }
        QVERIFY(obj);
        delete obj;
    }
}

// new loader, new plugin
void KLibLoaderTest::testWorking_KPluginLoader_KPluginFactory()
{
    KPluginLoader loader(s_kpluginFactoryModule);
    KPluginFactory* factory = loader.factory();
    if (!factory) {
        kWarning() << "error=" << loader.errorString();
        QVERIFY(factory);
    } else {
        QObject* obj = factory->create<QObject>();
        if (!obj) {
            kWarning() << "Error creating object";
        }
        QVERIFY(obj);
        delete obj;
    }
}

/* This file is part of the KDE libraries
    Copyright 2007 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kapplication_unittest.h"
#include "kapplication_unittest.moc"
#include <qtest_kde.h>

QTEST_KDEMAIN( KApplication_UnitTest, GUI )

#include <klibloader.h>
#include <kstandarddirs.h>
#include <QtGui/QClipboard>
#include <QtGui/QApplication>

#if 0

// This is a test for KApplication::slot_KLibLoader_hook_clearClipboard()
// But to work it would need a KApplication instance
// and the test takes forevery anyway.

// Created by the unittest in kdecore
static const char s_module[] = "libklibloadertestmodule";

void KApplication_UnitTest::testKLibLoaderUnload()
{
    setenv("KDE_DOUNLOAD", "1",  1);
    const QString libdir = QDir::currentPath() + "/../../lib";
    KGlobal::dirs()->addResourceDir( "module", libdir );

    const QString oldText = qApp->clipboard()->text();
    qApp->clipboard()->setText("KApplication_UnitTest");

    // First, do like KLibLoaderTest::testWorking()
    int error = 0;
    QObject* obj = KLibLoader::createInstance<QObject>( s_module, 0, QStringList(), &error );
    if ( error )
        kWarning() << "error=" << error << " lastErrorMessage=" << KLibLoader::self()->lastErrorMessage();
    QVERIFY( obj != 0 );

    // Now delete the object
    delete obj;

    // And wait
    QTest::qWait( 1000*10 + 20 ); // the value in KLibrary

    QCOMPARE(qApp->clipboard()->text(), QString());

    qApp->clipboard()->setText(oldText);
}
#endif

/* kate: tab-indents off; replace-tabs on; tab-width 4; remove-trailing-space on; encoding utf-8;*/
/*
This file is part of the KDE libraries
This file has been placed in the Public Domain.
*/

#undef QT_NO_CAST_FROM_ASCII

#include "ktemporaryfiletest.h"

#include "qtest_kde.h"

#include <QtCore/QDir>
#include <QCoreApplication>

#include "ktemporaryfile.h"

QTEST_MAIN( KTemporaryFileTest )

/*
Notes on these tests:

- I'm not testing anything that QTemporaryFile should handle, like the
working of setAutoRemove(). I suggest letting Trolltech handle making sure
those things work. These should only test KDE specific functionality.

*/

void KTemporaryFileTest::initTestCase()
{
    kdeTempDir = QDir::tempPath() + '/';
    componentName = QCoreApplication::instance()->applicationName();

    QDir qdir ( kdeTempDir );
    qdir.mkdir("ktempfiletest");
}

void KTemporaryFileTest::cleanupTestCase()
{
    QDir qdir ( kdeTempDir );
    qdir.rmdir("ktempfiletest");
}

// Test putting files in the default KDE temp directory
void KTemporaryFileTest::testKTemporaryFile()
{

    //Test basic placement
    QString first;
    {
        KTemporaryFile file;
        QVERIFY(file.open());
        QVERIFY(file.fileName().startsWith(kdeTempDir + componentName));
        QVERIFY(file.fileName().endsWith(QLatin1String(".tmp")));
        QVERIFY(QFile::exists(file.fileName()));
        first = file.fileName();
    }
    QVERIFY(!QFile::exists(first));

    // Test we don't get the same twice
    {
        KTemporaryFile file;
        QVERIFY(file.open());
        QVERIFY(first != file.fileName());
    }

    //Test relative subdirectory
    {
        KTemporaryFile file;
        file.setPrefix("ktempfiletest/");
        QVERIFY(file.open());
        QVERIFY(file.fileName().startsWith(kdeTempDir + "ktempfiletest/"));
        QVERIFY(file.fileName().endsWith(QLatin1String(".tmp")));
        QVERIFY(QFile::exists(file.fileName()));
    }

    //Test relative filename
    {
        KTemporaryFile file;
        file.setPrefix("spam");
        file.setSuffix("eggs");
        QVERIFY(file.open());
        QVERIFY(file.fileName().startsWith(kdeTempDir + "spam"));
        QVERIFY(file.fileName().endsWith(QLatin1String("eggs")));
        QVERIFY(QFile::exists(file.fileName()));
    }

    //Test suffix only
    {
        KTemporaryFile file;
        file.setSuffix("eggs");
        QVERIFY(file.open());
        QVERIFY(file.fileName().endsWith(QLatin1String("eggs")));
        QVERIFY(QFile::exists(file.fileName()));
    }

    //TODO: How to test outside of tmp when we can't be sure what
    //directories we have write access to?
}


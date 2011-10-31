/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include <QtTest/QtTest>
#include <qstandardpaths.h>
#include <qdebug.h>
#include <qstandardpaths.h>

//TESTED_CLASS=QStandardPaths
//TESTED_FILES=qstandardpaths.cpp

class tst_qstandardpaths : public QObject {
  Q_OBJECT

public:
    tst_qstandardpaths() {
    }
    virtual ~tst_qstandardpaths() {
    }

private slots:
    void testDefaultLocations();
    void testCustomLocations();
    void testLocateAll();

private:
    void setCustomLocations() {
        m_thisDir = QFile::decodeName(KDESRCDIR);
        m_thisDir.chop(1); // remove trailing slash!

        qputenv("XDG_CONFIG_HOME", QFile::encodeName(m_thisDir));
        QDir parent(m_thisDir);
        parent.cdUp();
        m_globalDir = parent.path(); // gives us a trailing slash
        qputenv("XDG_CONFIG_DIRS", QFile::encodeName(m_globalDir));
    }
    QString m_thisDir;
    QString m_globalDir;
};

void tst_qstandardpaths::testDefaultLocations()
{
#ifndef Q_OS_WIN
    qputenv("XDG_CONFIG_HOME", QByteArray());
    qputenv("XDG_CONFIG_DIRS", QByteArray());
    const QString expectedConfHome = QDir::homePath() + QString::fromLatin1("/.config");
    QCOMPARE(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation), expectedConfHome);
    const QStringList confDirs = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
    QCOMPARE(confDirs.count(), 2);
    QVERIFY(confDirs.contains(expectedConfHome));

    qputenv("XDG_DATA_HOME", QByteArray());
    qputenv("XDG_DATA_DIRS", QByteArray());
    const QStringList genericDataDirs = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    QCOMPARE(genericDataDirs.count(), 3);
    const QString expectedDataHome = QDir::homePath() + QString::fromLatin1("/.local/share");
    QCOMPARE(genericDataDirs.at(0), expectedDataHome);
    QCOMPARE(genericDataDirs.at(1), QString::fromLatin1("/usr/local/share"));
    QCOMPARE(genericDataDirs.at(2), QString::fromLatin1("/usr/share"));
#endif
}

void tst_qstandardpaths::testCustomLocations()
{
#ifndef Q_OS_WIN
    setCustomLocations();

    // test writableLocation()
    QCOMPARE(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation), m_thisDir);

    // test locate()
    const QString thisFileName = QString::fromLatin1("tst_qstandardpaths.cpp");
    QVERIFY(QFile::exists(m_thisDir + '/' + thisFileName));
    const QString thisFile = QStandardPaths::locate(QStandardPaths::ConfigLocation, thisFileName);
    QVERIFY(!thisFile.isEmpty());
    QVERIFY(thisFile.endsWith(thisFileName));

    const QString dir = QStandardPaths::locate(QStandardPaths::ConfigLocation, QString::fromLatin1("../autotests"), QStandardPaths::LocateDirectory);
    QVERIFY(!dir.isEmpty());
    const QString thisDirAsFile = QStandardPaths::locate(QStandardPaths::ConfigLocation, QString::fromLatin1("../tests")); // the default is LocateFile
    QVERIFY(thisDirAsFile.isEmpty()); // not a file

    const QStringList dirs = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
    QCOMPARE(dirs, QStringList() << m_thisDir << m_globalDir);
#endif
}

// We really need QTemporaryDir for this test...

void tst_qstandardpaths::testLocateAll()
{
    const QStringList appsDirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "applications", QStandardPaths::LocateDirectory);
    //qDebug() << appsDirs;
    Q_FOREACH(const QString &dir, appsDirs) {
        QVERIFY2(dir.endsWith(QLatin1String("/share/applications")), qPrintable(dir));
    }

#ifndef Q_OS_WIN
    setCustomLocations();
    const QStringList allFiles = QStandardPaths::locateAll(QStandardPaths::ConfigLocation, "CMakeLists.txt");
    QCOMPARE(allFiles.first(), QString(m_thisDir + QString::fromLatin1("/CMakeLists.txt")));
#endif
}

QTEST_MAIN(tst_qstandardpaths)

#include "tst_qstandardpaths.moc"

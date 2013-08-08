/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qurlpathinfo.h>

class tst_QUrlPathInfo : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void path_data();
    void path();
};

void tst_QUrlPathInfo::path_data()
{
    QTest::addColumn<QString>("urlStr");
    QTest::addColumn<QString>("expectedPath");
    QTest::addColumn<QString>("expectedPathNoSlash");
    QTest::addColumn<QString>("expectedPathWithSlash");

    QTest::newRow("absoluteFile") << "file:///temp/tmp.txt" << "/temp/tmp.txt" << "/temp/tmp.txt" << "/temp/tmp.txt/";
    QTest::newRow("absoluteDir") << "file:///temp/" << "/temp/" << "/temp" << "/temp/";
    QTest::newRow("absoluteInRoot") << "file:///temp" << "/temp" << "/temp" << "/temp/";
    QTest::newRow("noPath") << "ftp://ftp.kde.org" << QString() << QString() << QString();
    QTest::newRow("relative") << "temp/" << "temp/" << "temp" << "temp/";
    QTest::newRow("relativeNoSlash") << "tmp.txt" << "tmp.txt" << "tmp.txt" << "tmp.txt/";
    QTest::newRow("encoded") << "print:/specials/Print%20To%20File%20(PDF%252FAcrobat)" << "/specials/Print To File (PDF%2FAcrobat)"
                             << "/specials/Print To File (PDF%2FAcrobat)" << "/specials/Print To File (PDF%2FAcrobat)/";
}

void tst_QUrlPathInfo::path()
{
    QFETCH(QString, urlStr);
    QFETCH(QString, expectedPath);
    QFETCH(QString, expectedPathNoSlash);
    QFETCH(QString, expectedPathWithSlash);

    const QUrl url(urlStr);
    const QUrlPathInfo info(url);
    QCOMPARE(info.path(), expectedPath);
    QCOMPARE(info.path(QUrlPathInfo::StripTrailingSlash), expectedPathNoSlash);
}

QTEST_MAIN(tst_QUrlPathInfo)

#include "tst_qurlpathinfo.moc"

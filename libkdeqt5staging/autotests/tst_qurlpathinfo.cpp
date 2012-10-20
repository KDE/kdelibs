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

Q_DECLARE_METATYPE(QUrlPathInfo::EqualsOptions)

class tst_QUrlPathInfo : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void directoryAndFileName_data();
    void directoryAndFileName();
    void setFileName_data();
    void setFileName();
    void path_data();
    void path();
    void addPath_data();
    void addPath();
    void adjustPath_data();
    void adjustPath();
    void equals_data();
    void equals();
};

void tst_QUrlPathInfo::directoryAndFileName_data()
{
    QTest::addColumn<QString>("urlStr");
    QTest::addColumn<QString>("expectedDirectory");
    QTest::addColumn<QString>("expectedDirectoryTrailingSlash");
    QTest::addColumn<QString>("expectedFileName");
    QTest::addColumn<QString>("expectedDirectoryUrl");

    QTest::newRow("absoluteFile") << "file:///temp/tmp.txt" << "/temp" << "/temp/" << "tmp.txt" << "file:///temp";
    QTest::newRow("absoluteDir") << "file:///temp/" << "/temp" << "/temp/" << QString() << "file:///temp";
    QTest::newRow("absoluteInRoot") << "file:///temp" << "/" << "/" << "temp" << "file:///";
    QTest::newRow("relative") << "temp/tmp.txt" << "temp" << "temp/" << "tmp.txt" << "temp";
    QTest::newRow("relativeNoSlash") << "tmp.txt" << QString() << QString() << "tmp.txt" << QString();
    QTest::newRow("encoded") << "print:/specials/Print%20To%20File%20(PDF%252FAcrobat)" << "/specials" << "/specials/" << "Print To File (PDF%2FAcrobat)" << "print:/specials";
}

void tst_QUrlPathInfo::directoryAndFileName()
{
    QFETCH(QString, urlStr);
    QFETCH(QString, expectedDirectory);
    QFETCH(QString, expectedDirectoryTrailingSlash);
    QFETCH(QString, expectedFileName);
    QFETCH(QString, expectedDirectoryUrl);

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    const QUrl url(urlStr);
#else
    const QUrl url = QUrl::fromEncoded(urlStr.toUtf8());
#endif
    QVERIFY(url.isValid());
    const QUrlPathInfo info(url);
    QCOMPARE(info.directory(), expectedDirectory);
    QCOMPARE(info.directory(QUrlPathInfo::AppendTrailingSlash), expectedDirectoryTrailingSlash);
    QCOMPARE(info.fileName(), expectedFileName);
    QCOMPARE(info.directoryUrl().toString(), expectedDirectoryUrl);
}

void tst_QUrlPathInfo::setFileName_data()
{
    QTest::addColumn<QString>("urlStr");
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QString>("expectedUrl");

    QTest::newRow("simple") << "foo://host/bar" << "blah" << "foo://host/blah";
    QTest::newRow("empty") << "foo://host/bar" << "" << "foo://host/";
}

void tst_QUrlPathInfo::setFileName()
{
    QFETCH(QString, urlStr);
    QFETCH(QString, fileName);
    QFETCH(QString, expectedUrl);

    const QUrl url(urlStr);
    QVERIFY(url.isValid());
    QUrlPathInfo info(url);
    info.setFileName(fileName);
    QCOMPARE(info.url().toString(), expectedUrl);
}

void tst_QUrlPathInfo::path_data()
{
    QTest::addColumn<QString>("urlStr");
    QTest::addColumn<QString>("expectedPath");
    QTest::addColumn<QString>("expectedPathNoSlash");
    QTest::addColumn<QString>("expectedPathWithSlash");

    QTest::newRow("absoluteFile") << "file:///temp/tmp.txt" << "/temp/tmp.txt" << "/temp/tmp.txt" << "/temp/tmp.txt/";
    QTest::newRow("absoluteDir") << "file:///temp/" << "/temp/" << "/temp" << "/temp/";
    QTest::newRow("absoluteInRoot") << "file:///temp" << "/temp" << "/temp" << "/temp/";
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

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    const QUrl url(urlStr);
#else
    const QUrl url = QUrl::fromEncoded(urlStr.toUtf8());
#endif
    const QUrlPathInfo info(url);
    QCOMPARE(info.path(), expectedPath);
    QCOMPARE(info.path(QUrlPathInfo::StripTrailingSlash), expectedPathNoSlash);
    QCOMPARE(info.path(QUrlPathInfo::AppendTrailingSlash), expectedPathWithSlash);
}


void tst_QUrlPathInfo::addPath_data()
{
    QTest::addColumn<QString>("initialUrl");
    QTest::addColumn<QString>("toAdd");
    QTest::addColumn<QString>("expectedUrl");

    QTest::newRow("absoluteDir+fileName") << "file:///temp" << "tmp.txt" << "file:///temp/tmp.txt";
    QTest::newRow("absoluteDir+/fileName") << "file:///temp" << "/tmp.txt" << "file:///temp/tmp.txt";
    QTest::newRow("absoluteDir/+fileName") << "file:///temp/" << "tmp.txt" << "file:///temp/tmp.txt";
    QTest::newRow("absoluteDir/+/fileName") << "file:///temp/" << "/tmp.txt" << "file:///temp/tmp.txt";

    QTest::newRow("absoluteDir+subdir/fileName") << "file:///temp/" << "/temp/tmp.txt" << "file:///temp/temp/tmp.txt";
    QTest::newRow("absoluteInRoot+/") << "file:///" << "/" << "file:///";
    QTest::newRow("relative") << "temp" << "foo" << "temp/foo";
}

void tst_QUrlPathInfo::addPath()
{
    QFETCH(QString, initialUrl);
    QFETCH(QString, toAdd);
    QFETCH(QString, expectedUrl);

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    const QUrl url(initialUrl);
#else
    const QUrl url = QUrl::fromEncoded(initialUrl.toUtf8());
#endif
    QUrlPathInfo info(url);
    info.addPath(toAdd);
    QCOMPARE(info.url().toString(), expectedUrl);
}

void tst_QUrlPathInfo::adjustPath_data()
{
    path_data();
}

void tst_QUrlPathInfo::adjustPath()
{
    QFETCH(QString, urlStr);
    QFETCH(QString, expectedPath);
    QFETCH(QString, expectedPathNoSlash);
    QFETCH(QString, expectedPathWithSlash);

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    const QUrl url(urlStr);
#else
    const QUrl url = QUrl::fromEncoded(urlStr.toUtf8());
#endif
    QUrlPathInfo info(url);

    // Go via QUrlPathInfo::path in order to get decoded paths
    QCOMPARE(QUrlPathInfo(info.url()).path(), expectedPath);
    QCOMPARE(QUrlPathInfo(info.url(QUrlPathInfo::StripTrailingSlash)).path(), expectedPathNoSlash);
    QCOMPARE(QUrlPathInfo(info.url(QUrlPathInfo::AppendTrailingSlash)).path(), expectedPathWithSlash);
    info.adjustPath(QUrlPathInfo::StripTrailingSlash);
    QCOMPARE(info.path(), expectedPathNoSlash);
    info.adjustPath(QUrlPathInfo::AppendTrailingSlash);
    QCOMPARE(info.path(), expectedPathWithSlash);
}

void tst_QUrlPathInfo::equals_data()
{
    QTest::addColumn<QString>("urlStr1");
    QTest::addColumn<QString>("urlStr2");
    QTest::addColumn<QUrlPathInfo::EqualsOptions>("options");
    QTest::addColumn<bool>("expectedResult");

    const QUrlPathInfo::EqualsOptions strict(QUrlPathInfo::StrictComparison);
    const QUrlPathInfo::EqualsOptions noTrailing(QUrlPathInfo::CompareWithoutTrailingSlash);
    const QUrlPathInfo::EqualsOptions allowEmpty(QUrlPathInfo::CompareWithoutTrailingSlash | QUrlPathInfo::AllowEmptyPath);
    const QUrlPathInfo::EqualsOptions caseInsensitive(QUrlPathInfo::ComparePathsCaseInsensitively);
    const QUrlPathInfo::EqualsOptions caseInsensitiveNoTrailing(QUrlPathInfo::CompareWithoutTrailingSlash | QUrlPathInfo::ComparePathsCaseInsensitively);

    QTest::newRow("slash_diff") << "ftp://ftp.kde.org/dir" << "ftp://ftp.kde.org/dir/" << strict << false;
    QTest::newRow("slash_diff_ignore_slash") << "ftp://ftp.kde.org/dir" << "ftp://ftp.kde.org/dir/" << noTrailing << true;
    QTest::newRow("slash_diff_allow_empty") << "ftp://ftp.kde.org/dir" << "ftp://ftp.kde.org/dir/" << allowEmpty << true;
    QTest::newRow("slash_vs_empty") << "ftp://ftp.kde.org/" << "ftp://ftp.kde.org" << strict << false;
    QTest::newRow("slash_vs_empty_ignore_slash") << "ftp://ftp.kde.org/" << "ftp://ftp.kde.org" << noTrailing << false;
    QTest::newRow("slash_vs_empty_allow_empty") << "ftp://ftp.kde.org/" << "ftp://ftp.kde.org" << allowEmpty << true;
    QTest::newRow("same_case") << "ftp://ftp.kde.org/dir" << "ftp://ftp.kde.org/dir" << caseInsensitive << true;
    QTest::newRow("case_insensitive") << "ftp://ftp.kde.org/dir" << "ftp://ftp.kde.org/DIR" << caseInsensitive << true;
    QTest::newRow("case_insensitive_different") << "ftp://ftp.kde.org/dir/" << "ftp://ftp.kde.org/DIR" << caseInsensitive << false;
    QTest::newRow("case_insensitive_ignore_slash") << "ftp://ftp.kde.org/dir/" << "ftp://ftp.kde.org/DIR" << caseInsensitiveNoTrailing << true;

}

void tst_QUrlPathInfo::equals()
{
    QFETCH(QString, urlStr1);
    QFETCH(QString, urlStr2);
    QFETCH(QUrlPathInfo::EqualsOptions, options);
    QFETCH(bool, expectedResult);

    QUrl url1(urlStr1);
    QUrl url2(urlStr2);
    QCOMPARE(QUrlPathInfo(url1).equals(url2, options), expectedResult);
    QCOMPARE(QUrlPathInfo(url2).equals(url1, options), expectedResult);
}

QTEST_MAIN(tst_QUrlPathInfo)

#include "tst_qurlpathinfo.moc"

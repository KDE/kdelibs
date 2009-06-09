/*
 *  Copyright 2008 Friedrich W. H. Kossebau  <kossebau@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kaboutdatatest.h"

// test object
#include <kaboutdata.h>
// lib
#include <qtest_kde.h>
// Qt
#include <QtCore/QFile>
#include <QtCore/QTextStream>


static const char AppName[] =            "app";
static const char CatalogName[] =        "Catalog";
static const char ProgramName[] =        "ProgramName";
static const char ProgramIconName[] =    "program-icon";
static const char Version[] =            "Version";
static const char ShortDescription[] =   "ShortDescription";
static const char CopyrightStatement[] = "CopyrightStatement";
static const char Text[] =               "Text";
static const char HomePageAddress[] =    "http://test.no.where/";
static const char OrganizationDomain[] = "no.where";
static const char BugsEmailAddress[] =   "bugs@no.else";
static const char LicenseText[] =        "free to write, reading forbidden";
static const char LicenseFileName[] =    "testlicensefile";
static const char LicenseFileText[] =    "free to write, reading forbidden, in the file";


void KAboutDataTest::testConstructorWithDefaults()
{
    KAboutData aboutData(AppName, CatalogName, ki18n(ProgramName), Version );

    QCOMPARE( aboutData.appName(), QLatin1String(AppName) );
    QCOMPARE( aboutData.productName(), QLatin1String(AppName) );
    QCOMPARE( aboutData.programName(), ki18n(ProgramName).toString() );
    QCOMPARE( aboutData.programIconName(), QLatin1String(AppName) );
    QCOMPARE( aboutData.programLogo(), QVariant() );
    QCOMPARE( aboutData.organizationDomain(), QString("kde.org") );
    QCOMPARE( aboutData.version(), QString(Version) );
    QCOMPARE( aboutData.catalogName(), QString(CatalogName) );
    QCOMPARE( aboutData.homepage(), QString() );
    QCOMPARE( aboutData.bugAddress(), QString("submit@bugs.kde.org") );
    QVERIFY( aboutData.authors().isEmpty() );
    QVERIFY( aboutData.credits().isEmpty() );
    QVERIFY( aboutData.translators().isEmpty() );
    QCOMPARE( aboutData.otherText(), QString() );
// We don't know the default text, do we?
//     QCOMPARE( aboutData.licenseName(KAboutData::ShortName), QString(WarningText) );
    QVERIFY( aboutData.licenseName(KAboutData::ShortName) != QString() );
//     QCOMPARE( aboutData.licenseName(KAboutData::FullName), QString(WarningText) );
    QVERIFY( aboutData.licenseName(KAboutData::FullName) != QString() );
//     QCOMPARE( aboutData.license(), QString(WarningText) );
    QVERIFY( aboutData.license() != QString() );
    QCOMPARE( aboutData.licenses().count(), 1 );
// We don't know the default text, do we?
//     QCOMPARE( aboutData.licenses().at(0).name(KAboutData::ShortName), QString(WarningText) );
    QVERIFY( aboutData.licenses().at(0).name(KAboutData::ShortName) != QString() );
//     QCOMPARE( aboutData.licenses().at(0).name(KAboutData::FullName), QString(WarningText) );
    QVERIFY( aboutData.licenses().at(0).name(KAboutData::FullName) != QString() );
//     QCOMPARE( aboutData.licenses().at(0).text(), QString(WarningText) );
    QVERIFY( aboutData.licenses().at(0).text() != QString() );
    QCOMPARE( aboutData.copyrightStatement(), QString() );
    QCOMPARE( aboutData.shortDescription(), QString() );
    QCOMPARE( aboutData.customAuthorPlainText(), QString() );
    QCOMPARE( aboutData.customAuthorRichText(), QString() );
    QVERIFY( !aboutData.customAuthorTextEnabled() );
    //TODO: test internalVersion, internalProgramName, internalBugAddress
}

void KAboutDataTest::testConstructor()
{
    KAboutData aboutData(AppName, CatalogName, ki18n(ProgramName), Version,
                ki18n(ShortDescription), KAboutData::License_Unknown,
                ki18n(CopyrightStatement), ki18n(Text),
                HomePageAddress, BugsEmailAddress );

    QCOMPARE( aboutData.appName(), QLatin1String(AppName) );
    QCOMPARE( aboutData.productName(), QLatin1String(AppName) );
    QCOMPARE( aboutData.programName(), ki18n(ProgramName).toString() );
    QCOMPARE( aboutData.programIconName(), QLatin1String(AppName) );
    QCOMPARE( aboutData.programLogo(), QVariant() );
    QCOMPARE( aboutData.organizationDomain(), QString(OrganizationDomain) );
    QCOMPARE( aboutData.version(), QString(Version) );
    QCOMPARE( aboutData.catalogName(), QString(CatalogName) );
    QCOMPARE( aboutData.homepage(), QString(HomePageAddress) );
    QCOMPARE( aboutData.bugAddress(), QString(BugsEmailAddress) );
    QVERIFY( aboutData.authors().isEmpty() );
    QVERIFY( aboutData.credits().isEmpty() );
    QVERIFY( aboutData.translators().isEmpty() );
    QCOMPARE( aboutData.otherText(), QString(Text) );
//     QCOMPARE( aboutData.licenseName(KAboutData::ShortName), QString(WarningText) );
    QVERIFY( aboutData.licenseName(KAboutData::ShortName) != QString() );
//     QCOMPARE( aboutData.licenseName(KAboutData::FullName), QString(WarningText) );
    QVERIFY( aboutData.licenseName(KAboutData::FullName) != QString() );
//     QCOMPARE( aboutData.license(), QString(WarningText) );
    QVERIFY( aboutData.license() != QString() );
    QCOMPARE( aboutData.licenses().count(), 1 );
// We don't know the default text, do we?
//     QCOMPARE( aboutData.licenses().at(0).name(KAboutData::ShortName), QString(WarningText) );
    QVERIFY( aboutData.licenses().at(0).name(KAboutData::ShortName) != QString() );
//     QCOMPARE( aboutData.licenses().at(0).name(KAboutData::FullName), QString(WarningText) );
    QVERIFY( aboutData.licenses().at(0).name(KAboutData::FullName) != QString() );
//     QCOMPARE( aboutData.licenses().at(0).text(), QString(WarningText) );
    QVERIFY( aboutData.licenses().at(0).text() != QString() );
    QCOMPARE( aboutData.copyrightStatement(), ki18n(CopyrightStatement).toString() );
    QCOMPARE( aboutData.shortDescription(), ki18n(ShortDescription).toString() );
    QCOMPARE( aboutData.customAuthorPlainText(), QString() );
    QCOMPARE( aboutData.customAuthorRichText(), QString() );
    QVERIFY( !aboutData.customAuthorTextEnabled() );
    //TODO: test internalVersion, internalProgramName, internalBugAddress
}

void KAboutDataTest::testSetAddLicense()
{
    // prepare a file with a license text
    QFile licenseFile(LicenseFileName);
    licenseFile.open(QIODevice::WriteOnly);
    QTextStream licenseFileStream(&licenseFile);
    licenseFileStream << QLatin1String(LicenseFileText);
    licenseFile.close();

    const KLocalizedString copyrightStatement = ki18n(CopyrightStatement);
    const QString lineFeed( "\n\n" );

    KAboutData aboutData(AppName, CatalogName, ki18n(ProgramName), Version,
                ki18n(ShortDescription), KAboutData::License_Unknown,
                ki18n(CopyrightStatement), ki18n(Text),
                HomePageAddress, BugsEmailAddress );

    // set to GPL2
    aboutData.setLicense( KAboutData::License_GPL_V2 );

    QCOMPARE( aboutData.licenseName(KAboutData::ShortName), QString("GPL v2") );
    QCOMPARE( aboutData.licenseName(KAboutData::FullName), QString("GNU General Public License Version 2") );
//     QCOMPARE( aboutData.license(), QString(GPL2Text) );
    QVERIFY( aboutData.license() != QString() );
    QCOMPARE( aboutData.licenses().count(), 1 );
    QCOMPARE( aboutData.licenses().at(0).name(KAboutData::ShortName), QString("GPL v2") );
    QCOMPARE( aboutData.licenses().at(0).name(KAboutData::FullName), QString("GNU General Public License Version 2") );
//     QCOMPARE( aboutData.licenses().at(0).text(), QString(GPL2Text) );
    QVERIFY( aboutData.licenses().at(0).text() != QString() );

    // set to Unknown again
    aboutData.setLicense( KAboutData::License_Unknown );

//     QCOMPARE( aboutData.licenseName(KAboutData::ShortName), QString(WarningText) );
    QVERIFY( aboutData.licenseName(KAboutData::ShortName) != QString() );
//     QCOMPARE( aboutData.licenseName(KAboutData::FullName), QString(WarningText) );
    QVERIFY( aboutData.licenseName(KAboutData::FullName) != QString() );
//     QCOMPARE( aboutData.license(), QString(WarningText) );
    QVERIFY( aboutData.license() != QString() );
    QCOMPARE( aboutData.licenses().count(), 1 );
// We don't know the default text, do we?
//     QCOMPARE( aboutData.licenses().at(0).name(KAboutData::ShortName), QString(WarningText) );
    QVERIFY( aboutData.licenses().at(0).name(KAboutData::ShortName) != QString() );
//     QCOMPARE( aboutData.licenses().at(0).name(KAboutData::FullName), QString(WarningText) );
    QVERIFY( aboutData.licenses().at(0).name(KAboutData::FullName) != QString() );
//     QCOMPARE( aboutData.licenses().at(0).text(), QString(WarningText) );
    QVERIFY( aboutData.licenses().at(0).text() != QString() );

    // add GPL3
    aboutData.addLicense( KAboutData::License_GPL_V3 );

    QCOMPARE( aboutData.licenseName(KAboutData::ShortName), QString("GPL v3") );
    QCOMPARE( aboutData.licenseName(KAboutData::FullName), QString("GNU General Public License Version 3") );
//     QCOMPARE( aboutData.license(), QString(GPL3Text) );
    QVERIFY( aboutData.license() != QString() );
    QCOMPARE( aboutData.licenses().count(), 1 );
    QCOMPARE( aboutData.licenses().at(0).name(KAboutData::ShortName), QString("GPL v3") );
    QCOMPARE( aboutData.licenses().at(0).name(KAboutData::FullName), QString("GNU General Public License Version 3") );
//     QCOMPARE( aboutData.licenses().at(0).text(), QString(GPL3Text) );
    QVERIFY( aboutData.licenses().at(0).text() != QString() );

    // add GPL2, Custom and File
    aboutData.addLicense( KAboutData::License_GPL_V2 );
    aboutData.addLicenseText( ki18n(LicenseText) );
    aboutData.addLicenseTextFile( LicenseFileName );

    QCOMPARE( aboutData.licenseName(KAboutData::ShortName), QString("GPL v3") );
    QCOMPARE( aboutData.licenseName(KAboutData::FullName), QString("GNU General Public License Version 3") );
//     QCOMPARE( aboutData.license(), QString(GPL3Text) );
    QVERIFY( aboutData.license() != QString() );
    QCOMPARE( aboutData.licenses().count(), 4 );
    QCOMPARE( aboutData.licenses().at(0).name(KAboutData::ShortName), QString("GPL v3") );
    QCOMPARE( aboutData.licenses().at(0).name(KAboutData::FullName), QString("GNU General Public License Version 3") );
//     QCOMPARE( aboutData.licenses().at(0).text(), QString(GPL3Text) );
    QVERIFY( aboutData.licenses().at(0).text() != QString() );
    QCOMPARE( aboutData.licenses().at(1).name(KAboutData::ShortName), QString("GPL v2") );
    QCOMPARE( aboutData.licenses().at(1).name(KAboutData::FullName), QString("GNU General Public License Version 2") );
//     QCOMPARE( aboutData.licenses().at(1).text(), QString(GPL2Text) );
    QVERIFY( aboutData.licenses().at(1).text() != QString() );
    QCOMPARE( aboutData.licenses().at(2).name(KAboutData::ShortName), QString("Custom") );
    QCOMPARE( aboutData.licenses().at(2).name(KAboutData::FullName), QString("Custom") );
    QCOMPARE( aboutData.licenses().at(2).text(), ki18n(LicenseText).toString() );
    QCOMPARE( aboutData.licenses().at(3).name(KAboutData::ShortName), QString("Custom") );
    QCOMPARE( aboutData.licenses().at(3).name(KAboutData::FullName), QString("Custom") );
    QCOMPARE( aboutData.licenses().at(3).text(), copyrightStatement.toString()+lineFeed+ki18n(LicenseFileText).toString() );
}

void KAboutDataTest::testSetProgramIconName()
{
    const QLatin1String programIconName( ProgramIconName );

    KAboutData aboutData(AppName, CatalogName, ki18n(ProgramName), Version,
                ki18n(ShortDescription), KAboutData::License_Unknown,
                ki18n(CopyrightStatement), ki18n(Text),
                HomePageAddress, BugsEmailAddress );

    // set different iconname
    aboutData.setProgramIconName( programIconName );
    QCOMPARE( aboutData.programIconName(), programIconName );
}

void KAboutDataTest::testCopying()
{
    KAboutData aboutData(AppName, CatalogName, ki18n(ProgramName), Version,
                         ki18n(ShortDescription), KAboutData::License_GPL_V2);

    {
    KAboutData aboutData2(AppName, CatalogName, ki18n(ProgramName), Version,
                ki18n(ShortDescription), KAboutData::License_GPL_V3);
    aboutData2.addLicense(KAboutData::License_GPL_V2);
    aboutData = aboutData2;
    }
    QList<KAboutLicense> licenses = aboutData.licenses();
    QCOMPARE(licenses.count(), 2);
    QCOMPARE(licenses.at(0).key(), KAboutData::License_GPL_V3);
    // check it doesn't crash
    QVERIFY(!licenses.at(0).text().isEmpty());
    QCOMPARE(licenses.at(1).key(), KAboutData::License_GPL_V2);
    // check it doesn't crash
    QVERIFY(!licenses.at(1).text().isEmpty());
}

QTEST_KDEMAIN_CORE( KAboutDataTest )

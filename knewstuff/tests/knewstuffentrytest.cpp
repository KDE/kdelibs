/*
    This file is part of KNewStuff2.
    Copyright (c) 2008 Jeremy Whiting <jpwhiting@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

// unit test for entry

#include <QtTest>
#include <QtCore>

#include <qtest_kde.h>

#include "../knewstuff2/core/entry.h"
#include "../knewstuff2/core/ktranslatable.h"

const QString name = "Name";
const QString category = "Category";
const QString license = "GPLv2+";
const QString summary1 = "Summary of contents";
const QString summary2lang = "de";
const QString summary2 = "Translated summary of contents";
const QString version = "v0.1";
const int release = 1;
const QDate releasedate = QDate::currentDate();
const int rating = 80;
const int downloads = 5000;
const QString checksum = "BAADF00D";
const QString signature = "DEADBEEF";

class testEntry: public QObject
{
    Q_OBJECT
private slots:
    void testProperties();
    void testCopy();
    void testAssignment();
};

void testEntry::testProperties()
{
    KNS::Entry entry;
    entry.setName(KNS::KTranslatable(name));
    entry.setCategory(category);
    entry.setLicense(license);
    KNS::KTranslatable summary(summary1);
    summary.addString(summary2lang, summary2);
    entry.setSummary(summary);
    entry.setVersion(version);
    entry.setRelease(release);
    entry.setReleaseDate(releasedate);
    entry.setRating(rating);
    entry.setDownloads(downloads);
    entry.setChecksum(checksum);
    entry.setSignature(signature);

    QCOMPARE(entry.name().representation(), name);
    QCOMPARE(entry.category(), category);
    QCOMPARE(entry.license(), license);
    QCOMPARE(entry.summary().representation(), summary1);
    QCOMPARE(entry.summary().translated(summary2lang), summary2);
    QCOMPARE(entry.version(), version);
    QCOMPARE(entry.release(), release);
    QCOMPARE(entry.releaseDate(), releasedate);
    QCOMPARE(entry.rating(), rating);
    QCOMPARE(entry.downloads(), downloads);
    QCOMPARE(entry.checksum(), checksum);
    QCOMPARE(entry.signature(), signature);
}

void testEntry::testCopy()
{
    KNS::Entry entry;
    entry.setName(KNS::KTranslatable(name));
    entry.setCategory(category);
    entry.setLicense(license);
    KNS::KTranslatable summary(summary1);
    summary.addString(summary2lang, summary2);
    entry.setSummary(summary);
    entry.setVersion(version);
    entry.setRelease(release);
    entry.setReleaseDate(releasedate);
    entry.setRating(rating);
    entry.setDownloads(downloads);
    entry.setChecksum(checksum);
    entry.setSignature(signature);
    KNS::Entry entry2(entry);

    QCOMPARE(entry.name().representation(), entry2.name().representation());
    QCOMPARE(entry.category(), entry2.category());
    QCOMPARE(entry.license(), entry2.license());
    QCOMPARE(entry.summary().representation(), entry2.summary().representation());
    QCOMPARE(entry.summary().translated(summary2lang), entry2.summary().translated(summary2lang));
    QCOMPARE(entry.version(), entry2.version());
    QCOMPARE(entry.release(), entry2.release());
    QCOMPARE(entry.releaseDate(), entry2.releaseDate());
    QCOMPARE(entry.rating(), entry2.rating());
    QCOMPARE(entry.downloads(), entry2.downloads());
    QCOMPARE(entry.checksum(), entry2.checksum());
    QCOMPARE(entry.signature(), entry2.signature());
}

void testEntry::testAssignment()
{
    KNS::Entry entry;
    KNS::Entry entry2;;
    entry.setName(KNS::KTranslatable(name));
    entry.setCategory(category);
    entry.setLicense(license);
    KNS::KTranslatable summary(summary1);
    summary.addString(summary2lang, summary2);
    entry.setSummary(summary);
    entry.setVersion(version);
    entry.setRelease(release);
    entry.setReleaseDate(releasedate);
    entry.setRating(rating);
    entry.setDownloads(downloads);
    entry.setChecksum(checksum);
    entry.setSignature(signature);
    entry2 = entry;

    QCOMPARE(entry.name().representation(), entry2.name().representation());
    QCOMPARE(entry.category(), entry2.category());
    QCOMPARE(entry.license(), entry2.license());
    QCOMPARE(entry.summary().representation(), entry2.summary().representation());
    QCOMPARE(entry.summary().translated(summary2lang), entry2.summary().translated(summary2lang));
    QCOMPARE(entry.version(), entry2.version());
    QCOMPARE(entry.release(), entry2.release());
    QCOMPARE(entry.releaseDate(), entry2.releaseDate());
    QCOMPARE(entry.rating(), entry2.rating());
    QCOMPARE(entry.downloads(), entry2.downloads());
    QCOMPARE(entry.checksum(), entry2.checksum());
    QCOMPARE(entry.signature(), entry2.signature());
}

QTEST_KDEMAIN_CORE(testEntry)
#include "knewstuffentrytest.moc"

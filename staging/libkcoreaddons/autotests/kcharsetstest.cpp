/*
 * Copyright 2011  Romain Perier <bambi@kubuntu.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "kcharsetstest.h"

#include <kcharsets.h>
#include <QtCore/QString>
#include <QtCore/QDebug>
#include <QtTest/QtTest>

static bool encodingNameHasADescription(const QString &encodingName, const QStringList &descriptions)
{
    Q_FOREACH(QString description, descriptions)
        if (description.contains(encodingName))
            return true;
    return false;
}

void KCharsetsTest::testSingleton()
{
    QVERIFY(KCharsets::charsets() != NULL);
    QCOMPARE(KCharsets::charsets(), KCharsets::charsets());
}

void KCharsetsTest::testCodecForName()
{ 
    KCharsets *singleton = KCharsets::charsets();

    QCOMPARE(singleton->codecForName(QString::fromLatin1("ISO-8859-1"))->name(), QTextCodec::codecForName("ISO 8859-1")->name());
    QCOMPARE(singleton->codecForName(QString::fromLatin1("ISO 8859-1"))->name(), QTextCodec::codecForName("iso-8859-1")->name());
    QCOMPARE(singleton->codecForName(QString::fromLatin1("iso 8859-1"))->name(), QTextCodec::codecForName("iso-8859-1")->name());
    QCOMPARE(singleton->codecForName(QString::fromLatin1("utf-8"))->name(), QTextCodec::codecForName("UTF-8")->name());
    QCOMPARE(singleton->codecForName(QString::fromLatin1("utf8"))->name(), QTextCodec::codecForName("UTF-8")->name());
    QCOMPARE(singleton->codecForName(QString::fromLatin1("UTF8"))->name(), QTextCodec::codecForName("UTF-8")->name());

    QCOMPARE(singleton->codecForName(QString::fromLatin1(""))->name(), QTextCodec::codecForLocale()->name());
    QCOMPARE(singleton->codecForName(QString::fromLatin1("fake encoding"))->name(), QTextCodec::codecForName("iso-8859-1")->name());
}

void KCharsetsTest::testFromEntity()
{
    KCharsets *singleton = KCharsets::charsets();

    QCOMPARE(singleton->fromEntity(QString::fromLatin1("&#1234")), QChar(1234));
    QCOMPARE(singleton->fromEntity(QString::fromLatin1("&#x1234")), QChar(0x1234));
    QCOMPARE(singleton->fromEntity(QString::fromLatin1("lt")), QChar::fromLatin1('<'));
    QCOMPARE(singleton->fromEntity(QString::fromLatin1("gt")), QChar::fromLatin1('>'));
    QCOMPARE(singleton->fromEntity(QString::fromLatin1("quot")), QChar::fromLatin1('"'));
    QCOMPARE(singleton->fromEntity(QString::fromLatin1("amp")), QChar::fromLatin1('&'));
    QCOMPARE(singleton->fromEntity(QString::fromLatin1("apos")), QChar::fromLatin1('\''));
}

void KCharsetsTest::testToEntity()
{
    KCharsets *singleton = KCharsets::charsets();

    // Fix KCharsets::toEntity() first
}

void KCharsetsTest::testResolveEntities()
{
    KCharsets *singleton = KCharsets::charsets();

    QCOMPARE(singleton->resolveEntities(QString::fromLatin1("&quot;&apos;&lt;Hello &amp;World&gt;&apos;&quot;")),
        QString::fromLatin1("\"\'<Hello &World>\'\"")); 
}

void KCharsetsTest::testEncodingNames()
{
    KCharsets *singleton = KCharsets::charsets();

    QCOMPARE(singleton->availableEncodingNames().count(), singleton->descriptiveEncodingNames().count());

    Q_FOREACH(QString encodingName, singleton->availableEncodingNames()) {
        bool ok = false;

        if (encodingName == QString::fromLatin1("ucs2") || encodingName == QString::fromLatin1("ISO 10646-UCS-2"))
            singleton->codecForName(QString::fromLatin1("UTF-16"), ok);
        else if (encodingName == QString::fromLatin1("utf7"))
            continue;
        else
            singleton->codecForName(encodingName, ok);

        if (!ok) {
            qDebug() << "Error:" << encodingName << "not found";
            QVERIFY(NULL);
	}
        QVERIFY( encodingNameHasADescription(encodingName, singleton->descriptiveEncodingNames()) );
        QVERIFY( !singleton->descriptionForEncoding(encodingName).isEmpty() );
        QCOMPARE( singleton->encodingForName( singleton->descriptionForEncoding(encodingName) ), encodingName );
    }
}


QTEST_MAIN(KCharsetsTest)

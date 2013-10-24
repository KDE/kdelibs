/*
Copyright 2013 Aurélien Gâteau <agateau@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <ktranscripttest.h>

#include <QtTest/QtTest>

#include <QDebug>

#include <ktranscript_p.h>

QTEST_MAIN(KTranscriptTest)

extern "C"
{
    typedef KTranscript *(*InitFunc)();
}

KTranscriptTest::KTranscriptTest()
: m_transcript(0)
{
}

// FIXME: Duplicated from klocalizedstring.cpp. Should be shared.
static QString findKTranscriptPlugin()
{
    QStringList nameFilters;
    QString pluginSubdir = QLatin1String("kf5");
    QString pluginName = QLatin1String("ktranscript");
    nameFilters.append(pluginName + QLatin1String(".*"));
    Q_FOREACH (const QString &dirPath, QCoreApplication::libraryPaths()) {
        QString dirPathKf = dirPath + QLatin1Char('/') + pluginSubdir;
        if (!QDir(dirPathKf).entryList(nameFilters).isEmpty()) {
            return dirPathKf + QLatin1Char('/') + pluginName;
        }
    }
    return QString();
}

void KTranscriptTest::initTestCase()
{
    QString pluginPath = findKTranscriptPlugin();
    QVERIFY(!pluginPath.isEmpty());

    m_library.setFileName(pluginPath);
}

void KTranscriptTest::init()
{
    QVERIFY(m_library.load());
    InitFunc initf = (InitFunc) m_library.resolve("load_transcript");
    QVERIFY(initf);
    m_transcript = initf();
    QVERIFY(m_transcript);
}

void KTranscriptTest::cleanup()
{
    // load_transcript() returns a Q_GLOBAL_STATIC instance. The only way to
    // ensure we get a new instance is to unload the library.
    QVERIFY(m_library.unload());
    m_transcript = 0;
}

void KTranscriptTest::test_data()
{
    QTest::addColumn<QVariantList>("argv");
    QTest::addColumn<QString>("expected");

    QTest::newRow("test_basic") << (QVariantList() << "test_basic" << "foo") << "foo bar";
    QTest::newRow("test_msgtrf") << (QVariantList() << "test_msgtrf") << "ordinary";
}

void KTranscriptTest::test()
{
    QFETCH(QVariantList, argv);
    QFETCH(QString, expected);

    QString language = "fr";
    QString country = "fr";
    QString msgctxt = "msgctxt";
    QHash<QString, QString> dynamicContext;
    QString msgid = "msgid";
    QStringList subs;
    QList<QVariant> values;
    QString ordinaryTranslation = "ordinary";

    QString testJs = QFINDTESTDATA("test.js");
    QList<QStringList> modules;
    modules << (QStringList() << testJs << language);

    QString error;
    bool fallback;
    QString result = m_transcript->eval(
        argv, language, country,
        msgctxt, dynamicContext, msgid,
        subs, values, ordinaryTranslation,
        modules, error, fallback);

    if (!error.isEmpty()) {
        QFAIL(qPrintable(error));
    }
    QVERIFY(!fallback);
    QCOMPARE(result, expected);
}

#include <ktranscripttest.h>

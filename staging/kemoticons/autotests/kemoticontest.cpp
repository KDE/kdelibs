/*
    Tests for Kopete::Message::parseEmoticons

    Copyright (c) 2004      by Richard Smith          <kde@metafoo.co.uk>
    Copyright (c) 2005      by Duncan Mac-Vicar       <duncan@kde.org>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kemoticontest.h"
#include <QtTest/QtTest>

#include <stdlib.h>

#include <QDir>
#include <QFile>
#include <QTextStream>

#include <QDebug>
#include <kemoticons.h>

QTEST_MAIN(KEmoticonTest)

/*
  There are three sets of tests, the Kopete 0.7 baseline with tests that were
  working properly in Kopete 0.7.x. When these fail it's a real regression.

  The second set are those known to work in the current codebase.
  The last set is the set with tests that are known to fail right now.

   the name convention is working|broken-number.input|output
*/

void KEmoticonTest::testEmoticonParser()
{
    KEmoticonsTheme emo = KEmoticons().theme("kde4");
    QString basePath = QFINDTESTDATA("emoticon-parser-testcases");
    QVERIFY(!basePath.isEmpty());
    QDir testCasesDir(basePath);

    QStringList inputFileNames = testCasesDir.entryList(QStringList(QLatin1String("*.input")));
    Q_FOREACH (const QString& fileName, inputFileNames) {
        // qDebug() << "testcase: " << fileName;
        QString outputFileName = fileName;
        outputFileName.replace("input", "output");
        // open the input file
        QFile inputFile(basePath + QString::fromLatin1("/") + fileName);
        QFile expectedFile(basePath + QString::fromLatin1("/") + outputFileName);
        // check if the expected output file exists
        // if it doesn't, skip the testcase
        if (! expectedFile.exists()) {
            QSKIP("Warning! expected output for testcase not found. Skiping testcase");
            continue;
        }
        if (inputFile.open(QIODevice::ReadOnly) && expectedFile.open(QIODevice::ReadOnly)) {
            const QString inputData = QString::fromLatin1(inputFile.readAll().constData());
            const QString expectedData = QString::fromLatin1(expectedFile.readAll().constData());

            inputFile.close();
            expectedFile.close();

            const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "emoticons/kde4/smile.png").remove("smile.png");
            if (path.isEmpty())
                QSKIP("Emoticons not installed, skipping. kdebase-runtime needed.");

            QString result = emo.parseEmoticons(inputData, KEmoticonsTheme::RelaxedParse | KEmoticonsTheme::SkipHTML).replace(path, QString());

            // qDebug() << "Parse result: " << result;

            // HACK to know the test case we applied, concatenate testcase name to both
            // input and expected string. WIll remove when I can add some sort of metadata
            // to a CHECK so debug its origin testcase
            //result = fileName + QString::fromLatin1(": ") + result;
            //expectedData = fileName + QString::fromLatin1(": ") + expectedData;
            // if the test case begins with broken, we expect it to fail, then use XFAIL
            // otherwise use CHECK
            if (fileName.section("-", 0, 0) == QString::fromLatin1("broken")) {
                // qDebug() << "checking known-broken testcase: " << fileName;
                QEXPECT_FAIL("", "Checking know-broken testcase", Continue);
                QCOMPARE(result, expectedData);
            } else {
                // qDebug() << "checking known-working testcase: " << fileName;
                QCOMPARE(result, expectedData);
            }
        } else {
            QSKIP("Warning! can't open testcase files. Skiping testcase");
            continue;
        }
    }

}

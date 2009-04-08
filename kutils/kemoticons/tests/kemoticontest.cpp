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

// QTestLib for KDE
#include "kemoticontest.h"
#include "kemoticontest.moc"
#include <qtest_kde.h>

#include <stdlib.h>

#include <QDir>
#include <QFile>
#include <QTextStream>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kemoticons.h>

QTEST_KDEMAIN(KEmoticonTest, GUI)

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
    QString basePath = QString::fromLatin1(SRCDIR) + QString::fromLatin1("/emoticon-parser-testcases");
    QDir testCasesDir(basePath);

    QStringList inputFileNames = testCasesDir.entryList(QStringList(QLatin1String("*.input")));
    Q_FOREACH (const QString& fileName, inputFileNames) {
        kDebug() << "testcase: " << fileName;
        QString outputFileName = fileName;
        outputFileName.replace("input", "output");
        // open the input file
        QFile inputFile(basePath + QString::fromLatin1("/") + fileName);
        QFile expectedFile(basePath + QString::fromLatin1("/") + outputFileName);
        // check if the expected output file exists
        // if it doesn't, skip the testcase
        if (! expectedFile.exists()) {
            QSKIP("Warning! expected output for testcase not found. Skiping testcase", SkipSingle);
            continue;
        }
        if (inputFile.open(QIODevice::ReadOnly) && expectedFile.open(QIODevice::ReadOnly)) {
            QString inputData;
            QString expectedData;
            inputData = QString(inputFile.readAll());
            expectedData = QString(expectedFile.readAll());

            inputFile.close();
            expectedFile.close();

            QString path = KGlobal::dirs()->findResource("emoticons", "kde4/smile.png").remove("smile.png");

            QString result = emo.parseEmoticons(inputData, KEmoticonsTheme::RelaxedParse | KEmoticonsTheme::SkipHTML).replace(path, QString());

            kDebug() << "Parse result: " << result;

            // HACK to know the test case we applied, concatenate testcase name to both
            // input and expected string. WIll remove when I can add some sort of metadata
            // to a CHECK so debug its origin testcase
            //result = fileName + QString::fromLatin1(": ") + result;
            //expectedData = fileName + QString::fromLatin1(": ") + expectedData;
            // if the test case begins with broken, we expect it to fail, then use XFAIL
            // otherwise use CHECK
            if (fileName.section("-", 0, 0) == QString::fromLatin1("broken")) {
                kDebug() << "checking known-broken testcase: " << fileName;
                QEXPECT_FAIL("", "Checking know-broken testcase", Continue);
                QCOMPARE(result, expectedData);
            } else {
                kDebug() << "checking known-working testcase: " << fileName;
                QCOMPARE(result, expectedData);
            }
        } else {
            QSKIP("Warning! can't open testcase files. Skiping testcase", SkipSingle);
            continue;
        }
    }

}

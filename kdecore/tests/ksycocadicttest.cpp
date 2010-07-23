/* This file is part of the KDE project
   Copyright (C) 2007 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <ksycoca.h>
#include <QBuffer>
#include <kservicetype.h>
#include <ksycocadict.h>
#include <kdebug.h>
#include "qtest_kde.h"

class KSycocaDictTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testStandardDict();
    //void testExtensionDict();
private:
    void add(KSycocaDict& dict, const QString& key, const QString& name)
    {
        KServiceType::Ptr ptr = KServiceType::serviceType(name);
        if (!ptr)
            kWarning() << "serviceType not found" << name;
        dict.add(key, KSycocaEntry::Ptr::staticCast(ptr));
    }
};

QTEST_KDEMAIN_CORE(KSycocaDictTest)

// Standard use of KSycocaDict: mapping entry name to entry
void KSycocaDictTest::testStandardDict()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll ); // needed for KMimeType...

  QBENCHMARK {
    QByteArray buffer;
    QStringList mimeTypes;
    mimeTypes << "KUriFilter/Plugin"
              << "KDataTool"
              << "ThumbCreator"
              << "KScan/KScanDialog"
              << "Browser/View"
              << "Plasma/Applet"
              << "Plasma/Runner";
    {
        KSycocaDict dict;
        foreach(const QString& str, mimeTypes) {
            add(dict, str, str);
        }
        dict.remove("ThumbCreator"); // just to test remove
        add(dict, "ThumbCreator", "ThumbCreator");
        QCOMPARE((int)dict.count(), mimeTypes.count());
        QDataStream saveStream(&buffer, QIODevice::WriteOnly);
        dict.save(saveStream);
    }

    QDataStream stream(buffer);
    KSycocaDict loadingDict(&stream, 0);
    int offset = loadingDict.find_string("Browser/View");
    QVERIFY(offset > 0);
    QCOMPARE(offset, KServiceType::serviceType("Browser/View")->offset());
    foreach(const QString& str, mimeTypes) {
        int offset = loadingDict.find_string(str);
        QVERIFY(offset > 0);
        QCOMPARE(offset, KServiceType::serviceType(str)->offset());
    }
    offset = loadingDict.find_string("doesnotexist");
    QCOMPARE(offset, 0); // could be non 0 according to the docs, too; if non 0, we should check that the pointed mimetype doesn't have this name.
  }
}

// New use of KSycocaDict: mapping extension to mimetype; kind of a qmultihash
//   Not used anymore -> test disabled
#if 0
void KSycocaDictTest::testExtensionDict()
{
    if ( !KSycoca::isAvailable() )
        QSKIP( "ksycoca not available", SkipAll ); // needed for KMimeType...

  /*QBENCHMARK*/ {
    QByteArray buffer;
    QStringList extensions;
    extensions << "txt"
               << "xml"
               << "pdf"
               << "php"
               << "doc";
    QStringList mimeTypes;
    mimeTypes << "text/plain"
              << "application/xml"
              << "application/pdf"
              << "application/x-php"
              << "application/msword";
    {
        KSycocaDict dict;
        for(int i = 0; i < mimeTypes.count(); ++i) {
            add(dict, extensions[i], mimeTypes[i]);
        }
        dict.remove("php"); // just to test remove
        add(dict, "php", "application/x-php");
        // Now add another value for "doc"
        add(dict, "doc", "text/plain");
        QCOMPARE((int)dict.count(), mimeTypes.count()+1);
        QDataStream saveStream(&buffer, QIODevice::WriteOnly);
        dict.save(saveStream);
    }

    QDataStream stream(buffer);
    KSycocaDict loadingDict(&stream, 0);
    int offset = loadingDict.find_string("txt");
    QVERIFY(offset > 0);
    QCOMPARE(offset, KMimeType::mimeType("text/plain")->offset());
    for(int i = 0; i < mimeTypes.count(); ++i) {
        int offset = loadingDict.find_string(extensions[i]);
        QVERIFY(offset > 0);
        QCOMPARE(offset, KMimeType::mimeType(mimeTypes[i])->offset());
        if (extensions[i] != "doc") {
            QList<int> offsetList = loadingDict.findMultiString(extensions[i]);
            QCOMPARE(offsetList.count(), 1);
            QCOMPARE(offsetList.first(), offset);
        }
    }
    QList<int> offsetList = loadingDict.findMultiString("doc");
    QCOMPARE(offsetList.count(), 2);
    QCOMPARE(offsetList[0], KMimeType::mimeType("application/msword")->offset());
    QCOMPARE(offsetList[1], KMimeType::mimeType("text/plain")->offset());
  }
}
#endif

#include "ksycocadicttest.moc"

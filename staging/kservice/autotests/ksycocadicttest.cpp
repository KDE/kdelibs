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
#include <QtTest>
#include <QDebug>
#include <kservicetype.h>
#include <ksycocadict_p.h>

class KSycocaDictTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::enableTestMode(true);
    }
    void testStandardDict();
    //void testExtensionDict();
private:
    void add(KSycocaDict& dict, const QString& key, const QString& name)
    {
        KServiceType::Ptr ptr = KServiceType::serviceType(name);
        if (!ptr)
            qWarning() << "serviceType not found" << name;
        dict.add(key, KSycocaEntry::Ptr::staticCast(ptr));
    }
};

QTEST_MAIN(KSycocaDictTest)

// Standard use of KSycocaDict: mapping entry name to entry
void KSycocaDictTest::testStandardDict()
{
    if (!KServiceType::serviceType("KCModule"))
        QSKIP( "Missing servicetypes" );

    QStringList serviceTypes;
    serviceTypes << "KUriFilter/Plugin"
                 << "KDataTool"
                 << "KCModule"
                 << "KScan/KScanDialog"
                 << "Browser/View";

    if (KServiceType::serviceType("Plasma/Applet")) {
        serviceTypes << "Plasma/Applet"
                     << "Plasma/Runner";
    }

  QBENCHMARK {
    QByteArray buffer;
    {
        KSycocaDict dict;
        foreach(const QString& str, serviceTypes) {
            add(dict, str, str);
        }
        dict.remove("KCModule"); // just to test remove
        add(dict, "KCModule", "KCModule");
        QCOMPARE((int)dict.count(), serviceTypes.count());
        QDataStream saveStream(&buffer, QIODevice::WriteOnly);
        dict.save(saveStream);
    }

    QDataStream stream(buffer);
    KSycocaDict loadingDict(&stream, 0);
    int offset = loadingDict.find_string("Browser/View");
    QVERIFY(offset > 0);
    QCOMPARE(offset, KServiceType::serviceType("Browser/View")->offset());
    foreach(const QString& str, serviceTypes) {
        int offset = loadingDict.find_string(str);
        QVERIFY(offset > 0);
        QCOMPARE(offset, KServiceType::serviceType(str)->offset());
    }
    offset = loadingDict.find_string("doesnotexist");
    // TODO QCOMPARE(offset, 0); // could be non 0 according to the docs, too; if non 0, we should check that the pointed mimetype doesn't have this name.
  }
}

#include "ksycocadicttest.moc"

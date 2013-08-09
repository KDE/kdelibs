/* This file is part of the KDE project
   Copyright (C) 2010 Peter Penz <peter.penz19@gmail.com>

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

#include "kfilemetainfotest.h"
#include <kiotesthelper.h>

#include <kdirlister.h>
#include <kfilemetainfo.h>
#include <qtest.h>

#include <config-kde4support.h>

#include <QtCore/QThread>


/**
 * Reads the meta-data of a given file inside a thread.
 * Used to test KFileMetaInfo for reentrancy issues.
 */
class KFileMetaInfoThread : public QThread
{
public:
    KFileMetaInfoThread(const QString& fileName);

protected:
    virtual void run();

private:
    QString m_fileName;
};

KFileMetaInfoThread::KFileMetaInfoThread(const QString &fileName) :
    m_fileName(fileName)
{
}

void KFileMetaInfoThread::run()
{
    KFileMetaInfo fileMetaInfo(m_fileName);
}



QTEST_MAIN(KFileMetaInfoTest)

void KFileMetaInfoTest::initTestCase()
{
    m_exitCount = 0;
}

void KFileMetaInfoTest::testMetaInfo()
{
    const QString file = m_tempDir.path() + "/testfilename";
    createTestFile(file);

    KFileMetaInfo fileMetaInfo(file);
#if ! KIO_NO_STRIGI
    QVERIFY(fileMetaInfo.isValid());

    const KFileMetaInfoItem& item = fileMetaInfo.item("http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#fileName");
    QCOMPARE(item.value().toString(), QString("testfilename"));
#endif
}

void KFileMetaInfoTest::testReentrancy()
{
    const QString file = m_tempDir.path() + "/testfilename";
    createTestFile(file);

    m_exitCount = 20;
    for (int i = 0; i < m_exitCount; ++i) {
        QThread* thread = new KFileMetaInfoThread(file);
        thread->setParent(this);
        connect(thread, SIGNAL(finished()), this, SLOT(exitLoop()));
        thread->start();
    }

    m_eventLoop.exec();
}

void KFileMetaInfoTest::exitLoop()
{
    --m_exitCount;
    if (m_exitCount <= 0) {
        m_eventLoop.quit();
    }
}


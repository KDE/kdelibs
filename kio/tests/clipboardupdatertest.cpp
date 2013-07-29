/* This file is part of KDE
    Copyright (c) 2013 Dawit Alemayehu <adawit@kde.org>

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

#include <qtest_kde.h>

#include "clipboardupdatertest.h"
#include "kiotesthelper.h"
#include "clipboardupdater_p.h"

#include <kio/job.h>
#include <kio/paste.h>
#include <kio/copyjob.h>
#include <kio/deletejob.h>

#include <kdebug.h>
#include <ktempdir.h>

#include <QClipboard>
#include <QApplication>
#include <QMimeData>


QTEST_KDEMAIN( ClipboardUpdaterTest, GUI )

using namespace KIO;

static KUrl::List tempFiles(const KTempDir& dir, const QString& baseName, int count = 3)
{
    KUrl::List urls;
    const QString path = dir.name();
    for (int i = 1; i < count+1; ++i) {
        const QString file = (path + baseName + QString::number(i));
        urls << KUrl(file);
        createTestFile(file);
    }
    return urls;
}

void ClipboardUpdaterTest::testPasteAfterRenameFiles()
{
    KTempDir dir;
    const KUrl::List urls = tempFiles(dir, QLatin1String("rfile"));

    QClipboard* clipboard = QApplication::clipboard();
    QMimeData* mimeData = new QMimeData();
    urls.populateMimeData(mimeData);
    clipboard->setMimeData(mimeData);

    Q_FOREACH(const KUrl& url, urls) {
        KUrl newUrl = url;
        newUrl.setFileName(url.fileName() + QLatin1String("_renamed"));
        KIO::SimpleJob* job = KIO::rename(url, newUrl, KIO::HideProgressInfo);
        QVERIFY(job->exec());
    }

    const QString pasteDir = dir.name() + QLatin1String("pastedir");
    createTestDirectory(pasteDir, NoSymlink);
    KIO::Job* job = KIO::pasteClipboard(KUrl(pasteDir), 0);
    QVERIFY(job->exec());
    QVERIFY(job->error() == 0);
}

void ClipboardUpdaterTest::testPasteAfterMoveFile()
{
    KTempDir dir;
    const KUrl::List urls = tempFiles(dir, QLatin1String("mfile"), 1);

    QClipboard* clipboard = QApplication::clipboard();
    QMimeData* mimeData = new QMimeData();
    urls.populateMimeData(mimeData);
    clipboard->setMimeData(mimeData);

    const QString moveDir = dir.name() + QLatin1String("movedir/");
    createTestDirectory(moveDir, NoSymlink);
    const KUrl srcUrl = urls.first();
    KUrl destUrl (moveDir);
    destUrl.setFileName(srcUrl.fileName());
    KIO::FileCopyJob* mJob = KIO::file_move(srcUrl, destUrl, -1, KIO::HideProgressInfo);
    QVERIFY(mJob->exec());

    const QString pasteDir = dir.name() + QLatin1String("pastedir");
    createTestDirectory(pasteDir, NoSymlink);
    KIO::Job* job = KIO::pasteClipboard(KUrl(pasteDir), 0);
    QVERIFY(job->exec());
    QVERIFY(job->error() == 0);
}

void ClipboardUpdaterTest::testPasteAfterMoveFiles()
{
    KTempDir dir;
    const KUrl::List urls = tempFiles(dir, QLatin1String("mfile"));

    QClipboard* clipboard = QApplication::clipboard();
    QMimeData* mimeData = new QMimeData();
    urls.populateMimeData(mimeData);
    clipboard->setMimeData(mimeData);

    const QString moveDir = dir.name() + QLatin1String("movedir");
    createTestDirectory(moveDir, NoSymlink);
    KIO::CopyJob* mJob = KIO::move(urls, KUrl(moveDir), KIO::HideProgressInfo);
    QVERIFY(mJob->exec());

    const QString pasteDir = dir.name() + QLatin1String("pastedir");
    createTestDirectory(pasteDir, NoSymlink);
    KIO::Job* job = KIO::pasteClipboard(KUrl(pasteDir), 0);
    QVERIFY(job->exec());
    QVERIFY(job->error() == 0);
}

void ClipboardUpdaterTest::testPasteAfterDeleteFile()
{
    KTempDir dir;
    const KUrl::List urls = tempFiles(dir, QLatin1String("dfile"), 1);

    QClipboard* clipboard = QApplication::clipboard();
    QMimeData* mimeData = new QMimeData();
    urls.populateMimeData(mimeData);
    clipboard->setMimeData(mimeData);

    SimpleJob* sJob = KIO::file_delete(urls.first(), KIO::HideProgressInfo);
    QVERIFY(sJob->exec());

    QVERIFY(!clipboard->mimeData()->hasUrls());

    const QString pasteDir = dir.name() + QLatin1String("pastedir");
    createTestDirectory(pasteDir, NoSymlink);
    KIO::Job* job = KIO::pasteClipboard(KUrl(pasteDir), 0);
    QVERIFY(!job);
}

void ClipboardUpdaterTest::testPasteAfterDeleteFiles()
{
    KTempDir dir;
    const KUrl::List urls = tempFiles(dir, QLatin1String("dfile"));

    QClipboard* clipboard = QApplication::clipboard();
    QMimeData* mimeData = new QMimeData();
    urls.populateMimeData(mimeData);
    clipboard->setMimeData(mimeData);

    DeleteJob* dJob = KIO::del(urls, KIO::HideProgressInfo);
    QVERIFY(dJob->exec());

    QVERIFY(!clipboard->mimeData()->hasUrls());

    const QString pasteDir = dir.name() + QLatin1String("pastedir");
    createTestDirectory(pasteDir, NoSymlink);
    KIO::Job* job = KIO::pasteClipboard(KUrl(pasteDir), 0);
    QVERIFY(!job);
}

#include "clipboardupdatertest.moc"

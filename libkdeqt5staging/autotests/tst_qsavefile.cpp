/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <../../kde_qt5_compat.h>
#include <QtTest/QtTest>
#include <qcoreapplication.h>
#include <qstring.h>
#include <qtemporaryfile.h>
#include <qtemporarydir.h>
#include <qfile.h>
#include <qsavefile.h>
#include <qdir.h>
#include <qset.h>

#if defined(Q_OS_WIN)
# include <windows.h>
#endif

class tst_QSaveFile : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void transactionalWrite();
    void autoFlush();
    void transactionalWriteNoPermissions();
    void transactionalWriteCanceled();
    void transactionalWriteErrorRenaming();
};

void tst_QSaveFile::transactionalWrite()
{
    const QString targetFile = QString::fromLatin1("outfile");
    QFile::remove(targetFile);
    QSaveFile file(targetFile);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QVERIFY(file.isOpen());
    QCOMPARE(file.fileName(), targetFile);
    QVERIFY(!QFile::exists(targetFile));

    QTextStream ts(&file);
    ts << "This is test data one.\n";
    ts.flush();
    QCOMPARE(file.error(), QFile::NoError);
    QVERIFY(!QFile::exists(targetFile));

    QVERIFY(file.commit());
    QVERIFY(QFile::exists(targetFile));
    QCOMPARE(file.fileName(), targetFile);

    // Check that we can reuse a QSaveFile object
    // (and test the case of an existing target file)
    QVERIFY(file.open(QIODevice::WriteOnly));
    QCOMPARE(file.write("Hello"), 5LL);
    QVERIFY(file.commit());

    QFile reader(targetFile);
    QVERIFY(reader.open(QIODevice::ReadOnly));
    QCOMPARE(QString::fromLatin1(reader.readAll().constData()), QString::fromLatin1("Hello"));
    reader.close();

    QFile::remove(targetFile);
}

void tst_QSaveFile::autoFlush()
{
    const QString targetFile = QString::fromLatin1("outfile");
    QFile::remove(targetFile);
    QSaveFile file(targetFile);
    QVERIFY(file.open(QIODevice::WriteOnly));

    QTextStream ts(&file);
    ts << "Auto-flush.";
    // no flush
    QVERIFY(file.commit()); // close will emit aboutToClose, which will flush the stream
    QFile reader(targetFile);
    QVERIFY(reader.open(QIODevice::ReadOnly));
    QCOMPARE(QString::fromLatin1(reader.readAll().constData()), QString::fromLatin1("Auto-flush."));
    QFile::remove(targetFile);
}

void tst_QSaveFile::transactionalWriteNoPermissions()
{
#ifdef Q_OS_UNIX
    if (::geteuid() == 0)
        QSKIP_PORTING("not valid running this test as root", SkipAll);

    // You can write into /dev/zero, but you can't create a /dev/zero.XXXXXX temp file.
    QSaveFile file("/dev/zero");
    if (!QDir("/dev").exists())
        QSKIP_PORTING("/dev doesn't exist on this system", SkipAll);

    QVERIFY(!file.open(QIODevice::WriteOnly));
    QCOMPARE((int)file.error(), (int)QFile::OpenError);
    QVERIFY(!file.commit());
#endif
}

void tst_QSaveFile::transactionalWriteCanceled()
{
    const QString targetFile = QString::fromLatin1("outfile");
    QFile::remove(targetFile);
    QSaveFile file(targetFile);
    QVERIFY(file.open(QIODevice::WriteOnly));

    QTextStream ts(&file);
    ts << "This writing operation will soon be canceled.\n";
    ts.flush();
    QCOMPARE(file.error(), QFile::NoError);
    QVERIFY(!QFile::exists(targetFile));

    // We change our mind, let's abort writing
    file.cancelWriting();

    QVERIFY(!file.commit());

    QVERIFY(!QFile::exists(targetFile)); // temp file was discarded
    QCOMPARE(file.fileName(), targetFile);
}

void tst_QSaveFile::transactionalWriteErrorRenaming()
{
    QTemporaryDir dir;
    const QString targetFile = dir.path() + QString::fromLatin1("/outfile");
    QSaveFile file(targetFile);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QCOMPARE(file.write("Hello"), qint64(5));
    QVERIFY(!QFile::exists(targetFile));

#ifdef Q_OS_UNIX
    QFile dirAsFile(dir.path()); // yay, I have to use QFile to change a dir's permissions...
    QVERIFY(dirAsFile.setPermissions(QFile::Permissions(0))); // no permissions
#else
    QVERIFY(file.setPermissions(QFile::ReadOwner));
#endif

    QVERIFY(!file.commit());
    QVERIFY(!QFile::exists(targetFile)); // renaming failed
    QCOMPARE(file.error(), QFile::RenameError);

    // Restore permissions so that the cleanup can happen
#ifdef Q_OS_UNIX
    QVERIFY(dirAsFile.setPermissions(QFile::Permissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner)));
#else
    QVERIFY(file.setPermissions(QFile::ReadOwner | QFile::WriteOwner));
#endif
}

QTEST_MAIN(tst_QSaveFile)
#include "tst_qsavefile.moc"

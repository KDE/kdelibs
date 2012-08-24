/*  This file is part of the KDE project
    Copyright (C) 2012 Cyril Oblikov <munknex@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2+ as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef FILESTRANSFERDIALOG_H
#define FILESTRANSFERDIALOG_H

#include <QObject>
#include <QTimer>
#include <QVariant>
#include <QHash>
#include "filehelpers.h"
#include "kio/copyjob.h" // for CopyInfo

class FileModel;

namespace KIO
{

class FilesTransferDialog : public QObject
{
    Q_OBJECT

public:
    explicit FilesTransferDialog(QObject *parent = 0);
    Q_INVOKABLE QObject* normalModel();
    Q_INVOKABLE QObject* skippedModel();
    Q_INVOKABLE QObject* unreadableModel();
    Q_INVOKABLE QObject* disappearedModel();
    
signals:
    // for QML
    void totalAmountChanged(QVariant amounts);
    void finishedAmountChanged(QVariant amounts);
    void unfinishedAmountChanged(QVariant amounts);
    void errorsAmountChanged(QVariant amounts);
    void skippedAmountChanged(QVariant amounts);

    void windowTitleChanged(QString title);

    // for CopyJob
    void cancel();
    void suspend();
    void resume();

    void skipped(int id);
    void retried(int id);
    
public slots:
    // from CopyJob
    void gotAllFiles(QList<int> fids, QList<KIO::CopyInfo> files);
    void gotProcessedAmount(KJob*, KJob::Unit unit, qulonglong amount);
    void gotSkippedFile(int id);
    void gotRetriedFile(int id);
    void gotUnreadableFile(int id);
    void gotDisappearedFile(int id);
    void gotProcessedSizeOfFile(int fid, qulonglong size);
    void gotSpeed(KJob*, unsigned long speed);
    void nothingToProcess();
    void resendSignals();

    // for manual resolution
    void skipFile(const QString &modelName, int id);
    void retryFile(const QString &modelName, int id);
    void skipAllFiles(const QString &modelName);
    void retryAllFiles(const QString &modelName);

    // for unreadable files
    void changeRights(const QString &modelName, bool always);

    // for normal files
    void clearFinished(const QString &modelName);

    // for skipped files
    void clearSkipped(const QString &modelName);

private:
    FileModel* findModelByFileId(int id);
    void updateWindowTitle();
    
    // helpers
    void emitFinishedAmountChanged();
    void emitUnfinishedAmountChanged();
    void emitSkippedAmountChanged();
    void emitErrorsAmountChanged();

    qulonglong m_totalSize;
    qulonglong m_speed;
    qulonglong m_finishedFilesCount;
    qulonglong m_finishedFilesSize;
    qulonglong m_unfinishedFilesCount;
    qulonglong m_unfinishedFilesSize;
    qulonglong m_skippedFilesCount;
    qulonglong m_skippedFilesSize;
    qulonglong m_errorsFilesCount;
    qulonglong m_errorsFilesSize;
    FileModel *m_normalModel;
    FileModel *m_skippedModel;
    FileModel *m_unreadableModel;
    FileModel *m_disappearedModel;
};

}

#endif // FILESTRANSFERDIALOG_H

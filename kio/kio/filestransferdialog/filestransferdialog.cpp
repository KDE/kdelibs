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

#include "filestransferdialog.h"

#include <QtCore>
#include "kglobal.h"
#include "klocale.h"
#include "kdebug.h"
#include "filemodel.h"

using namespace KIO;

static QVariant convertAmounts(qulonglong files, qulonglong bytes, qreal ratio)
{
    QVariantList args;
    args << i18np("1 file", "%1 files", files);
    args << KGlobal::locale()->formatByteSize(bytes);
    args << QVariant::fromValue(static_cast<float>(ratio));
    return args;
}


FilesTransferDialog::FilesTransferDialog(QObject *parent) :
    QObject(parent),
    m_totalSize(0),
    m_speed(0),
    m_finishedFilesCount(0),
    m_finishedFilesSize(0),
    m_unfinishedFilesCount(0),
    m_unfinishedFilesSize(0),
    m_skippedFilesCount(0),
    m_skippedFilesSize(0),
    m_errorsFilesCount(0),
    m_errorsFilesSize(0),
    m_normalModel(new FileModel(this)),
    m_skippedModel(new FileModel(this)),
    m_unreadableModel(new FileModel(this)),
    m_disappearedModel(new FileModel(this))
{
    m_normalModel->setObjectName("normalModel");
    m_skippedModel->setObjectName("skippedModel");
    m_unreadableModel->setObjectName("unreadableModel");
    m_disappearedModel->setObjectName("disappearedModel");
}

QObject* FilesTransferDialog::normalModel()
{
    return m_normalModel;
}

QObject* FilesTransferDialog::skippedModel()
{
    return m_skippedModel;
}

QObject* FilesTransferDialog::unreadableModel()
{
    return m_unreadableModel;
}

QObject* FilesTransferDialog::disappearedModel()
{
    return m_disappearedModel;
}

void FilesTransferDialog::emitFinishedAmountChanged()
{
    emit finishedAmountChanged(convertAmounts(m_finishedFilesCount, m_finishedFilesSize, qreal(m_finishedFilesSize)/qreal(m_totalSize)));
}

void FilesTransferDialog::emitUnfinishedAmountChanged()
{
    emit unfinishedAmountChanged(convertAmounts(m_unfinishedFilesCount, m_unfinishedFilesSize, qreal(m_unfinishedFilesSize)/qreal(m_totalSize)));
}

void FilesTransferDialog::emitSkippedAmountChanged()
{
    emit skippedAmountChanged(convertAmounts(m_skippedFilesCount, m_skippedFilesSize, qreal(m_skippedFilesSize)/qreal(m_totalSize)));
}

void FilesTransferDialog::emitErrorsAmountChanged()
{
    emit errorsAmountChanged(convertAmounts(m_errorsFilesCount, m_errorsFilesSize, qreal(m_errorsFilesSize)/qreal(m_totalSize)));
}

void FilesTransferDialog::gotAllFiles(QList<int> fids, QList<KIO::CopyInfo> files)
{
    kDebug() << "count:(%d, %d)" << fids.count() << files.count();
    m_totalSize = 0;
    for (int i = 0; i < fids.count(); i++) {
        m_totalSize += files[i].size;
        m_normalModel->enqueue(FileItem(fids[i], files[i], FileHelper::Unfinished));
    }

    m_unfinishedFilesCount = fids.count();
    m_unfinishedFilesSize = m_totalSize;
    kDebug() << "files" << fids.count() << "bytes:" << m_totalSize;
    emit finishedAmountChanged(convertAmounts(0, 0, 0));
    emit unfinishedAmountChanged(convertAmounts(m_unfinishedFilesCount, m_unfinishedFilesSize, 1));
    emit errorsAmountChanged(convertAmounts(0, 0, 0));
    emit skippedAmountChanged(convertAmounts(0, 0, 0));
    updateWindowTitle();
}

void FilesTransferDialog::gotProcessedAmount(KJob*, KJob::Unit unit, qulonglong amount)
{
    switch (unit) {
        case KJob::Files: {
            qulonglong delta = amount - m_finishedFilesCount;
            m_finishedFilesCount = amount;
            m_unfinishedFilesCount -= delta;
            kDebug() << "files:" << amount;
            break;
        }
        case KJob::Bytes: {
            qulonglong delta = amount - m_finishedFilesSize;
            m_finishedFilesSize = amount;
            m_unfinishedFilesSize -= delta;
            kDebug() << "unit:" << unit << "amount:" << amount << "unfinishedSize" << m_unfinishedFilesSize;
            break;
        }
        case KJob::Errors:
        case KJob::Skipped:
            return;
    }

    emitFinishedAmountChanged();
    emitUnfinishedAmountChanged();
}

void FilesTransferDialog::gotProcessedFileRatio(int fid, qreal ratio)
{
    m_normalModel->updateFileProgress(fid, ratio, FileHelper::Finished);
}

FileModel* FilesTransferDialog::findModelByFileId(int id)
{
    FileModel *model = NULL;
    if (m_normalModel->contains(id)) {
        model = m_normalModel;
    } else if (m_skippedModel->contains(id)) {
        model = m_skippedModel;
    } else if (m_unreadableModel->contains(id)) {
        model = m_unreadableModel;
    } else if (m_disappearedModel->contains(id)) {
        model = m_disappearedModel;
    } else {
        model = NULL;
        kDebug() << "Error: file" << id << "was not found!";
    }

    return model;
}

void FilesTransferDialog::gotSkippedFile(int id)
{
    FileModel *model = findModelByFileId(id);
    if (model == NULL) {
        kDebug() << "Error: file" << id << "was not found!";
        return;
    }
    FileItem file = model->takeFile(id);
    if (file.isNull()) {
        kDebug() << "Error: trying to skip file with id:" << id;
        return;
    }

    if (model == m_normalModel) {
        m_unfinishedFilesCount -= 1;
        m_unfinishedFilesSize -= file.size.toLongLong();
        emitUnfinishedAmountChanged();
    } else {
        m_errorsFilesCount -= 1;
        m_errorsFilesSize -= file.size.toLongLong();
        emitErrorsAmountChanged();
    }

    m_skippedFilesCount++;
    m_skippedFilesSize += file.size.toLongLong();
    file.progress = 0;
    file.actions = FileHelper::Skipped;
    m_skippedModel->enqueue(file);
    emitSkippedAmountChanged();
}

void FilesTransferDialog::gotRetriedFile(int id)
{
    FileModel *model = findModelByFileId(id);
    if (model == NULL) {
        kDebug() << "Error: file" << id << "was not found!";
        return;
    }
    FileItem file = model->takeFile(id);
    if (file.isNull()) {
        kDebug() << "Error: trying to skip file with id:" << id;
        return;
    }

    if (model == m_skippedModel) {
        m_skippedFilesCount -= 1;
        m_skippedFilesSize -= file.size.toLongLong();
        emitSkippedAmountChanged();
    } else { // m_errorsModel
        m_errorsFilesCount -= 1;
        m_errorsFilesSize -= file.size.toLongLong();
        emitErrorsAmountChanged();
    }

    m_unfinishedFilesCount++;
    m_unfinishedFilesSize += file.size.toLongLong();
    file.progress = 0;
    file.actions = FileHelper::Unfinished;
    m_normalModel->enqueue(file);

    emitUnfinishedAmountChanged();
}

void FilesTransferDialog::gotUnreadableFile(int id)
{
    FileItem file = m_normalModel->takeFile(id);
    if (file.isNull()) {
        kDebug() << "Error: trying to take file with id:" << id;
        return;
    }

    m_unfinishedFilesCount -= 1;
    m_unfinishedFilesSize -= file.size.toLongLong();
    m_errorsFilesCount++;
    m_errorsFilesSize += file.size.toLongLong();
    file.progress = 0;
    file.actions = FileHelper::Unreadable;
    m_unreadableModel->enqueue(file);
    emitErrorsAmountChanged();
    emitUnfinishedAmountChanged();
}

void FilesTransferDialog::gotDisappearedFile(int id)
{
    FileItem file = m_normalModel->takeFile(id);
    if (file.isNull()) {
        kDebug() << "Error: trying to take file with id:" << id;
        return;
    }

    m_unfinishedFilesCount -= 1;
    m_unfinishedFilesSize -= file.size.toLongLong();
    m_errorsFilesCount++;
    m_errorsFilesSize += file.size.toLongLong();
    file.progress = 0;
    file.actions = FileHelper::Disappeared;
    m_disappearedModel->enqueue(file);
    emitErrorsAmountChanged();
    emitUnfinishedAmountChanged();
}

void FilesTransferDialog::nothingToProcess()
{
    kDebug() << "nothingToProcess";
    m_normalModel->setFinished();
    updateWindowTitle();
}

void FilesTransferDialog::resendSignals()
{
    kDebug() << "resending signals";
    emitFinishedAmountChanged();
    emitUnfinishedAmountChanged();
    emitSkippedAmountChanged();
    emitErrorsAmountChanged();
    updateWindowTitle();
}

void FilesTransferDialog::gotSpeed(KJob*, unsigned long speed)
{
    kDebug() << "speed" << speed;
    m_speed = speed;
    updateWindowTitle();
}

void FilesTransferDialog::updateWindowTitle()
{
    QString title;
    
    if (m_unfinishedFilesCount == 0) {
        title = i18n("Task completed. %1 %2",
                     (m_skippedFilesCount == 0) ? "" : i18np("1 file skipped,", "%1 files skipped,", m_skippedFilesCount),
                     (m_errorsFilesCount == 0) ? "" : i18np("1 error", "%1 errors", m_errorsFilesCount));
    } else if (m_speed != 0) {
        title = i18n("%1 %2",
                         i18nc("Bytes per second", "%1/s", KGlobal::locale()->formatByteSize(1000 * m_speed)),
                         (m_speed != 0) ? i18n("(%1 remaning)", KGlobal::locale()->prettyFormatDuration(1000 * m_unfinishedFilesSize / m_speed)) : "");
    } else {
        title = "Copying";
    }
    emit windowTitleChanged(title);
}

// from QML

void FilesTransferDialog::skipFile(const QString &modelName, int id)
{
    FileModel *model = findChild<FileModel *>(modelName);
    if (model == NULL || !model->contains(id)) {
        kDebug() << "Error: no field:" << id << "in model:" << qPrintable(modelName);
        return;
    }
    // notify CopyJob
    emit skipped(id);
}

void FilesTransferDialog::retryFile(const QString &modelName, int id)
{
    Q_UNUSED(modelName);
    if (!m_skippedModel->contains(id)) {
        kDebug() << "Error: trying to retry file with id:" << id;
        return;
    }
    // notify CopyJob
    emit retried(id);
}

void FilesTransferDialog::skipAllFiles(const QString &modelName)
{
    FileModel *model = findChild<FileModel *>(modelName);
    if (model == NULL) {
        kDebug() << "Error: no such model:" << modelName;
        return;
    }
    QList<int> fids = model->allIDs();
    if (fids.isEmpty()) {
        kDebug() << "Error: model:" << modelName << "is empty";
        return;
    }
    foreach (int fid, fids) {
        emit skipped(fid);
    }
}

void FilesTransferDialog::retryAllFiles(const QString &modelName)
{
    FileModel *model = findChild<FileModel *>(modelName);
    if (model == NULL) {
        kDebug() << "Error: no such model:" << modelName;
        return;
    }
    QList<int> fids = model->allIDs();
    if (fids.isEmpty()) {
        kDebug() << "Error: model:" << modelName << "is empty";
        return;
    }
    foreach (int fid, fids) {
        emit retried(fid);
    }
}

void FilesTransferDialog::changeRights(const QString &modelName, bool always)
{

}

void FilesTransferDialog::clearFinished(const QString &modelName)
{
    m_normalModel->removeFinished();
}

void FilesTransferDialog::clearSkipped(const QString &modelName)
{

}

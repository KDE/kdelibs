#include "filestransferdialog.h"

#include <QtCore>
#include "kglobal.h"
#include "klocale.h"
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

void FilesTransferDialog::gotAllFiles(QList<int> fids, QList<KIO::CopyInfo> files)
{
    qDebug("FilesTransferDialog::gotAllFiles: count:(%d, %d)", fids.count(), files.count());
    m_totalSize = 0;
    for (int i = 0; i < fids.count(); i++) {
        m_totalSize += files[i].size;
        m_normalModel->enqueue(FileItem(fids[i], files[i], FileHelper::Unfinished));
    }

    m_unfinishedFilesCount = fids.count();
    m_unfinishedFilesSize = m_totalSize;
    qDebug("KIOConnect::gotAllFiles: files:%lld, bytes:%lld", static_cast<qulonglong>(fids.count()), m_totalSize);
    emit finishedAmountChanged(convertAmounts(0, 0, 0));
    emit unfinishedAmountChanged(convertAmounts(m_unfinishedFilesCount, m_unfinishedFilesSize, 1));
    emit errorsAmountChanged(convertAmounts(0, 0, 0));
    emit skippedAmountChanged(convertAmounts(0, 0, 0));
    updateWindowTitle();
}

void FilesTransferDialog::gotProcessedAmount(KJob*, KJob::Unit unit, qulonglong amount)
{
    if (unit == KJob::Files) {
        qulonglong delta = amount - m_finishedFilesCount;
        m_finishedFilesCount = amount;
        m_unfinishedFilesCount -= delta;
        qDebug("KIOConnect::gotProcessedAmount: files:%lld", amount);
    } else if (unit == KJob::Bytes) {
        qulonglong delta = amount - m_finishedFilesSize;
        m_finishedFilesSize = amount;
        m_unfinishedFilesSize -= delta;
        qDebug("KIOConnect::gotProcessedAmount: unit:%d, amount:%lld, unfSize%lld", unit, amount, m_unfinishedFilesSize);
    }

    emit finishedAmountChanged(convertAmounts(m_finishedFilesCount, m_finishedFilesSize, qreal(m_finishedFilesSize)/qreal(m_totalSize)));
    emit unfinishedAmountChanged(convertAmounts(m_unfinishedFilesCount, m_unfinishedFilesSize, qreal(m_unfinishedFilesSize)/qreal(m_totalSize)));
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
        qDebug("Error: KIOConnect::findModelByFileId: file %d was not found!", id);
    }

    return model;
}

void FilesTransferDialog::gotSkippedFile(int id)
{
    FileModel *model = findModelByFileId(id);
    if (model == NULL) {
        qDebug("Error: KIOConnect::gotSkippedFile: file %d was not found!", id);
        return;
    }
    FileItem file = model->takeFile(id);
    if (file.isNull()) {
        qDebug("Error: KIOConnect::gotSkippedFile: trying to skip file with id:%d", id);
        return;
    }

    if (model == m_normalModel) {
        m_unfinishedFilesCount -= 1;
        m_unfinishedFilesSize -= file.size.toLongLong();
        emit unfinishedAmountChanged(convertAmounts(m_unfinishedFilesCount, m_unfinishedFilesSize, qreal(m_unfinishedFilesSize)/qreal(m_totalSize)));
    } else {
        m_errorsFilesCount -= 1;
        m_errorsFilesSize -= file.size.toLongLong();
        emit errorsAmountChanged(convertAmounts(m_errorsFilesCount, m_errorsFilesSize, qreal(m_errorsFilesSize)/qreal(m_totalSize)));
    }

    m_skippedFilesCount += 1;
    m_skippedFilesSize += file.size.toLongLong();
    file.progress = 0;
    file.actions = FileHelper::Skipped;
    m_skippedModel->enqueue(file);
    emit skippedAmountChanged(convertAmounts(m_skippedFilesCount, m_skippedFilesSize, qreal(m_skippedFilesSize)/qreal(m_totalSize)));
}

void FilesTransferDialog::gotRetriedFile(int id)
{
    FileModel *model = findModelByFileId(id);
    if (model == NULL) {
        qDebug("Error: KIOConnect::gotRetriedFile: file %d was not found!", id);
        return;
    }
    FileItem file = model->takeFile(id);
    if (file.isNull()) {
        qDebug("Error: KIOConnect::gotRetriedFile: trying to skip file with id:%d", id);
        return;
    }

    if (model == m_skippedModel) {
        m_skippedFilesCount -= 1;
        m_skippedFilesSize -= file.size.toLongLong();
        emit skippedAmountChanged(convertAmounts(m_skippedFilesCount, m_skippedFilesSize, qreal(m_skippedFilesSize)/qreal(m_totalSize)));
    } else { // m_errorsModel
        m_errorsFilesCount -= 1;
        m_errorsFilesSize -= file.size.toLongLong();
        emit errorsAmountChanged(convertAmounts(m_errorsFilesCount, m_errorsFilesSize, qreal(m_errorsFilesSize)/qreal(m_totalSize)));
    }

    m_unfinishedFilesCount += 1;
    m_unfinishedFilesSize += file.size.toLongLong();
    file.progress = 0;
    file.actions = FileHelper::Unfinished;
    m_normalModel->enqueue(file);

    emit unfinishedAmountChanged(convertAmounts(m_unfinishedFilesCount, m_unfinishedFilesSize, qreal(m_unfinishedFilesSize)/qreal(m_totalSize)));
}

void FilesTransferDialog::gotUnreadableFile(int id)
{
    FileItem file = m_normalModel->takeFile(id);
    if (file.isNull()) {
        qDebug("Error: KIOConnect::gotUnreadableFile: trying to take file with id:%d", id);
        return;
    }

    m_unfinishedFilesCount -= 1;
    m_unfinishedFilesSize -= file.size.toLongLong();
    m_errorsFilesCount += 1;
    m_errorsFilesSize += file.size.toLongLong();
    file.progress = 0;
    file.actions = FileHelper::Unreadable;
    m_unreadableModel->enqueue(file);
    emit errorsAmountChanged(convertAmounts(m_errorsFilesCount, m_errorsFilesSize, qreal(m_errorsFilesSize)/qreal(m_totalSize)));
    emit unfinishedAmountChanged(convertAmounts(m_unfinishedFilesCount, m_unfinishedFilesSize, qreal(m_unfinishedFilesSize)/qreal(m_totalSize)));
}

void FilesTransferDialog::gotDisappearedFile(int id)
{
    FileItem file = m_normalModel->takeFile(id);
    if (file.isNull()) {
        qDebug("Error: KIOConnect::gotDisappearedFile: trying to take file with id:%d", id);
        return;
    }

    m_unfinishedFilesCount -= 1;
    m_unfinishedFilesSize -= file.size.toLongLong();
    m_errorsFilesCount += 1;
    m_errorsFilesSize += file.size.toLongLong();
    file.progress = 0;
    file.actions = FileHelper::Disappeared;
    m_disappearedModel->enqueue(file);
    emit errorsAmountChanged(convertAmounts(m_errorsFilesCount, m_errorsFilesSize, qreal(m_errorsFilesSize)/qreal(m_totalSize)));
    emit unfinishedAmountChanged(convertAmounts(m_unfinishedFilesCount, m_unfinishedFilesSize, qreal(m_unfinishedFilesSize)/qreal(m_totalSize)));
}

void FilesTransferDialog::nothingToProcess()
{
    qDebug("KIOConnect::nothingToProcess:");
    m_normalModel->setFinished();
    updateWindowTitle();
}

void FilesTransferDialog::resendSignals()
{
    qDebug("FilesTransferDialog::resendSignals:");
    emit finishedAmountChanged(convertAmounts(m_finishedFilesCount, m_finishedFilesSize, qreal(m_finishedFilesSize)/qreal(m_totalSize)));
    emit unfinishedAmountChanged(convertAmounts(m_unfinishedFilesCount, m_unfinishedFilesSize, qreal(m_unfinishedFilesSize)/qreal(m_totalSize)));
    emit skippedAmountChanged(convertAmounts(m_skippedFilesCount, m_skippedFilesSize, qreal(m_skippedFilesSize)/qreal(m_totalSize)));
    emit errorsAmountChanged(convertAmounts(m_errorsFilesCount, m_errorsFilesSize, qreal(m_errorsFilesSize)/qreal(m_totalSize)));
    updateWindowTitle();
}

void FilesTransferDialog::gotSpeed(KJob*, unsigned long speed)
{
    qDebug("FilesTransferDialog::gotSpeed: %u", speed);
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
        qDebug("Error: KIOConnect::skipFile: no fild:%d in model:%s", id, qPrintable(modelName));
        return;
    }
    // notify CopyJob
    emit skipped(id);
}

void FilesTransferDialog::retryFile(const QString &modelName, int id)
{
    Q_UNUSED(modelName);
    if (!m_skippedModel->contains(id)) {
        qDebug("Error: KIOConnect::retryFile: trying to retry file with id:%d", id);
        return;
    }
    // notify CopyJob
    emit retried(id);
}

void FilesTransferDialog::skipAllFiles(const QString &modelName)
{
    FileModel *model = findChild<FileModel *>(modelName);
    if (model == NULL) {
        qDebug("Error: KIOConnect::skipAllFiles: no such model: %s", qPrintable(modelName));
        return;
    }
    QList<int> fids = model->allIDs();
    if (fids.isEmpty()) {
        qDebug("Error: KIOConnect::skipAllFiles: model %s is empty", qPrintable(modelName));
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
        qDebug("Error: KIOConnect::retryAllFiles: no such model: %s", qPrintable(modelName));
        return;
    }
    QList<int> fids = model->allIDs();
    if (fids.isEmpty()) {
        qDebug("Error: KIOConnect::retryAllFiles: model %s is empty", qPrintable(modelName));
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

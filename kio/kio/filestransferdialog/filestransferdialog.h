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
    void gotProcessedFileRatio(int fid, qreal ratio);
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

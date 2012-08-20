#ifndef COPYJOBFAKE_H
#define COPYJOBFAKE_H

#include <QObject>
#include <QTimer>
#include <QHash>
#include "filehelpers.h"

enum ISSUE_TYPE {
    NONE,
    SKIPPED,
    UNREADABLE,
    DISAPPEARED,
    ALREADY_EXISTS
};

struct Issue
{
    Issue(int _id, ISSUE_TYPE _type): id(_id), type(_type) {}
    int id;
    ISSUE_TYPE type;
};

struct File
{
    File() {}
    File(int _id, QString _url, int _size) : id(_id), url(_url), size(_size) {}
    int id;
    QString url;
    qulonglong size;
};

class CopyJobFake : public QObject
{
    Q_OBJECT
public:
    explicit CopyJobFake(QObject *parent = 0);
    void loadFromFile(const QString &fileName);

signals:
    void allFiles(QList<int> fids, QList<QString> urls, QList<qulonglong> sizes);
    void totalAmount(FileHelper::Unit unit, qulonglong amount);
    void processedAmount(FileHelper::Unit unit, qulonglong amount);
    void processedFileRatio(int fileID, qreal ratio);
    void skippedFile(int id);
    void retriedFile(int id);
    void unreadableFile(int id);
    void disappearedFile(int id);
    void nothingToProcess();
    void speed(unsigned long speed);
    
public slots:
    void retryRequest(int id);
    void skipRequest(int id);

private slots:
    void chunkDone();

private:
    void processLine(int id, const QString &line);
    void appendNormalFile(const File &file, ISSUE_TYPE issueType = NONE);

    QTimer m_timer;
    qulonglong m_speed_lastChunksCount;
    qulonglong m_speed_lastChunksSize;
    qulonglong m_processingSize;
    qulonglong m_processedSize;
    qulonglong m_processedFilesCount;
    qulonglong m_total_size;
    QList<Issue> m_issues;
    QList<File> m_files;
    QHash<int, File> m_skipped;
    QHash<int, File> m_unreadable;
    QHash<int, File> m_disappeared;
};

#endif // COPYJOBFAKE_H

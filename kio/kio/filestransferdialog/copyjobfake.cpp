#include "copyjobfake.h"
#include <QtCore>

static const unsigned int MIN_FILE_SIZE = 1024;
static const unsigned int MAX_FILE_SIZE = 300 * 1024 * 1024;
static const unsigned int MIN_CHUNK_SIZE = 500 * 1024;
static const unsigned int MAX_CHUNK_SIZE = 1000 * 1024;
static const unsigned int SPEED_INTERVAL = 1000;
static const unsigned int TIMER_INTERVAL = 50;

CopyJobFake::CopyJobFake(QObject *parent) :
    QObject(parent)
{
    qsrand((uint)QTime::currentTime().msec());
    m_speed_lastChunksSize = 0;
    m_speed_lastChunksCount = 0;
    m_processingSize = 0;
    m_processedSize = 0;
    m_processedFilesCount = 0;
    m_total_size = 0;
}

static int rand_int(int from, int to) {
    return from + (qrand() % (to - from));
}

void CopyJobFake::processLine(int id, const QString &line) {
    if (line.startsWith("#"))
        return;

    ISSUE_TYPE type = NONE;
    QStringList values = line.split(" ", QString::SkipEmptyParts);
    if (values.count() == 2) {
        if (values[1] == "UNREADABLE") {
            type = UNREADABLE;
        } else if (values[1] == "DISAPPEARED") {
            type = DISAPPEARED;
        } else if (values[1] == "ALREADY_EXISTS") {
            type = ALREADY_EXISTS;
        } else {
            type = NONE;
        }
    }
    m_issues.append(Issue(id, type));
    int fileSize = rand_int(MIN_FILE_SIZE, MAX_FILE_SIZE);
    m_files.append(File(id, values[0], fileSize));
    m_total_size += fileSize;
}

void CopyJobFake::loadFromFile(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        qFatal("can not open file: %s", qPrintable(file.errorString()));

    int idCounter = 0;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        processLine(idCounter, line);
        idCounter++;
    }

    QList<int> fids;
    QList<QString> urls;
    QList<qulonglong> sizes;
    foreach (const File &file, m_files) {
        fids.append(file.id);
        urls.append(file.url);
        sizes.append(file.size);
    }
    emit allFiles(fids, urls, sizes);

    connect(&m_timer, SIGNAL(timeout()), SLOT(chunkDone()));
    m_timer.start(TIMER_INTERVAL);
}

void CopyJobFake::chunkDone()
{
    if (m_files.isEmpty()) {
        qDebug("CopyJobFake::chunkDone stopping timer");
        m_timer.stop();
        emit nothingToProcess();
        return;
    }

    int chunk =  rand_int(MIN_CHUNK_SIZE, MAX_CHUNK_SIZE);
    m_speed_lastChunksCount += 1;
    m_speed_lastChunksSize += chunk;

    if (m_speed_lastChunksCount * TIMER_INTERVAL >= SPEED_INTERVAL) {
        emit speed((m_speed_lastChunksSize / m_speed_lastChunksCount) / TIMER_INTERVAL);
        m_speed_lastChunksCount = 0;
        m_speed_lastChunksSize = 0;
    }

    m_processingSize += chunk;

    while (m_issues.first().type != NONE) {
        File file = m_files.takeFirst();
        Issue issue = m_issues.takeFirst();
        switch (issue.type) {
        case UNREADABLE:
            m_processingSize = 0;
            m_unreadable.insert(file.id, file);
            emit unreadableFile(file.id);
            break;
        case DISAPPEARED:
            m_processingSize = 0;
            m_disappeared.insert(file.id, file);
            emit disappearedFile(file.id);
            break;
        default:
            break;
        }
    }

    while (!m_files.isEmpty() && m_processingSize >= m_files.first().size) {
        File file = m_files.takeFirst();
        Issue issue = m_issues.takeFirst();
        if (issue.type != NONE) {
            m_processingSize = 0;
            break;
        }
        m_processingSize -= file.size;
        m_processedSize += file.size;
        m_processedFilesCount++;
    }

    emit processedAmount(FileHelper::Files, m_processedFilesCount);
    emit processedAmount(FileHelper::Bytes, m_processedSize + ((m_files.isEmpty()) ? 0 : m_processingSize) );
    if (!m_files.isEmpty()) {
        const File &file = m_files.first();
        emit processedFileRatio(file.id, static_cast<double>(m_processingSize) / static_cast<double>(file.size));
    }
}

void CopyJobFake::appendNormalFile(const File &file, ISSUE_TYPE issueType)
{
    m_files.append(file);
    m_issues.append(Issue(file.id, issueType));
}

void CopyJobFake::retryRequest(int id)
{
    qDebug("CopyJobFake::retryRequest id:%d", id);
    if (m_skipped.contains(id)) {
        appendNormalFile(m_skipped.take(id));
        qDebug("CopyJobFake::retryRequest retried skipped id:%d", id);
        emit retriedFile(id);
    } else if (m_unreadable.contains(id)) {
        appendNormalFile(m_unreadable.take(id));
        qDebug("CopyJobFake::retryRequest retried unreadable id:%d", id);
        emit retriedFile(id);
    } else if (m_disappeared.contains(id)) {
        appendNormalFile(m_disappeared.take(id));
        qDebug("CopyJobFake::retryRequest retried disappeared id:%d", id);
        emit retriedFile(id);
    }
    if (!m_timer.isActive()) {
        m_timer.start();
    }
}

void CopyJobFake::skipRequest(int id)
{
    File skipped;
    qDebug("CopyJobFake::skipRequest id:%d", id);
    for (int i = 0; i < m_files.size(); i++) {
        if (m_files[i].id == id) {
            m_skipped.insert(m_files[i].id, m_files[i]);
            skipped = m_files.takeAt(i);
            m_issues.removeAt(i);
            if (i == 0) {
                m_processingSize = 0;
            }
            qDebug("CopyJobFake::skipRequest skipped id:%d from %d index", id, i);
            emit skippedFile(id);
            break;
        }
    }

    if (m_unreadable.contains(id)) {
        File file = m_unreadable.take(id);
        m_skipped.insert(file.id, file);
        emit skippedFile(id);
    } else if (m_disappeared.contains(id)) {
        File file = m_disappeared.take(id);
        m_skipped.insert(file.id, file);
        emit skippedFile(id);
    }
}

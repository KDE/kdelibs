#include <numeric>

#include <QtDebug>
#include <QString>
#include <QtTest>
#include <QCoreApplication>
#include <QList>

#include <Job.h>
#include <JobCollection.h>
#include <JobSequence.h>
#include <ThreadWeaver.h>

//FIXME Wouldn't it be nice to be able to execute jobs in the local thread?
class AccumulateJob : public ThreadWeaver::Job {
public:
    explicit AccumulateJob()
        : m_count(0)
        , m_result(0)
    {
    }

    void setCount(quint64 count) {
        m_count = count;
    }

    quint64 result() const {
        return m_result;
    }

    void executeLocal() {
        run();
    }

protected:
    void run() {
        quint64 numbers[m_count];
        for(quint64 i = 0; i < m_count; ++i) {
            numbers[i] = i;
        }
        m_result = std::accumulate(numbers, numbers + m_count, 0);
    }

private:
    quint64 m_count;
    quint64 m_result;
};

class QueueBenchmarksTest : public QObject
{
    Q_OBJECT
    
public:
    QueueBenchmarksTest();
    
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void BaselineBenchmark();
    void BaselineBenchmark_data();
    void BaselineAsJobsBenchmark();
    void BaselineAsJobsBenchmark_data();
    void IndividualJobsBenchmark();
    void IndividualJobsBenchmark_data();
    void CollectionsBenchmark();
    void CollectionsBenchmark_data();
    void SequencesBenchmark();
    void SequencesBenchmark_data();

private:
    void defaultBenchmarkData(bool singleThreaded);
};

QueueBenchmarksTest::QueueBenchmarksTest()
{
}

void QueueBenchmarksTest::initTestCase()
{
}

void QueueBenchmarksTest::cleanupTestCase()
{
}

/** @brief BaselineBenchmark simply performs the same operations in a loop.
 *
 * The result amounts to what time the jobs used in the benchmark need to execute without queueing or thread
 * synchronization overhead. */
void QueueBenchmarksTest::BaselineBenchmark()
{
    QFETCH(int, m);
    QFETCH(int, c);
    QFETCH(int, b);
    QFETCH(int, t);
    const int n = c*b;
    Q_UNUSED(t); // in this case

    AccumulateJob jobs[n];
    for(int i = 0; i < n; ++i) {
        jobs[i].setCount(m);
    }

    //Actually, executeLocal needs to emit similar signals as execute(), to be comparable to the threaded variants
    QBENCHMARK {
        for(int i = 0; i < n; ++i) {
            jobs[i].executeLocal();
        }
    }
}



void QueueBenchmarksTest::BaselineBenchmark_data()
{
    defaultBenchmarkData(true);
}

void QueueBenchmarksTest::BaselineAsJobsBenchmark()
{
    QFETCH(int, m);
    QFETCH(int, c);
    QFETCH(int, b);
    QFETCH(int, t);
    const int n = c*b;
    Q_UNUSED(t); // in this case

    AccumulateJob jobs[n];
    for(int i = 0; i < n; ++i) {
        jobs[i].setCount(m);
    }

    //Actually, executeLocal needs to emit similar signals as execute(), to be comparable to the threaded variants
    QBENCHMARK {
        for(int i = 0; i < n; ++i) {
            jobs[i]();
        }
    }
}

void QueueBenchmarksTest::BaselineAsJobsBenchmark_data()
{
    defaultBenchmarkData(true);
}

void QueueBenchmarksTest::IndividualJobsBenchmark()
{
    QFETCH(int, m);
    QFETCH(int, c);
    QFETCH(int, b);
    QFETCH(int, t);
    const int n = c*b;

    ThreadWeaver::Weaver weaver;
    weaver.setMaximumNumberOfThreads(t);
    weaver.suspend();
    AccumulateJob jobs[n];
    for(int i = 0; i < n; ++i) {
        jobs[i].setCount(m);
        weaver.enqueue(&jobs[i]);
    }

    QBENCHMARK {
        weaver.resume();
        weaver.finish();
    }
}

void QueueBenchmarksTest::IndividualJobsBenchmark_data()
{
    defaultBenchmarkData(false);
}

void QueueBenchmarksTest::CollectionsBenchmark()
{
    QFETCH(int, m);
    QFETCH(int, c);
    QFETCH(int, b);
    QFETCH(int, t);
    const int n = c*b;

    ThreadWeaver::Weaver weaver;
    weaver.setMaximumNumberOfThreads(t);
    weaver.suspend();
    AccumulateJob jobs[n];


    QObject parent;
    qDebug() << b << "blocks" << c << "operations, queueing...";
    //queue the jobs blockwise as collections
    for (int block = 0; block < b; ++block) {
        ThreadWeaver::JobCollection* collection = new ThreadWeaver::JobCollection(&parent);
        for (int operation = 0; operation < c; ++operation) {
            const int index = block * b + operation;
            jobs[index].setCount(m);
            collection->addJob(&jobs[index]);
        }
        weaver.enqueue(collection);
    }

    qDebug() << b << "blocks" << c << "operations, executing...";
    QBENCHMARK {
        weaver.resume();
        weaver.finish();
    }
}

void QueueBenchmarksTest::CollectionsBenchmark_data()
{
    defaultBenchmarkData(false);
}

void QueueBenchmarksTest::SequencesBenchmark()
{
    QFETCH(int, m);
    QFETCH(int, c);
    QFETCH(int, b);
    QFETCH(int, t);
    const int n = c*b;

    ThreadWeaver::Weaver weaver;
    weaver.setMaximumNumberOfThreads(t);
    weaver.suspend();
    AccumulateJob jobs[n];


    QObject parent;
    qDebug() << b << "blocks" << c << "operations, queueing...";
    //queue the jobs blockwise as collections
    for (int block = 0; block < b; ++block) {
        ThreadWeaver::JobSequence* sequence = new ThreadWeaver::JobSequence(&parent);
        for (int operation = 0; operation < c; ++operation) {
            const int index = block * b + operation;
            jobs[index].setCount(m);
            sequence->addJob(&jobs[index]);
        }
        weaver.enqueue(sequence);
    }

    qDebug() << b << "blocks" << c << "operations, executing...";
    QBENCHMARK {
        weaver.resume();
        weaver.finish();
    }
}

void QueueBenchmarksTest::SequencesBenchmark_data()
{
    defaultBenchmarkData(false);
}

void QueueBenchmarksTest::defaultBenchmarkData(bool singleThreaded)
{
    QTest::addColumn<int>("m"); // number of quint64's to accumulate
    QTest::addColumn<int>("c"); // operations per block
    QTest::addColumn<int>("b"); // number of blocks, number of jobs is b*c
    QTest::addColumn<int>("t"); // number of worker threads

    const QList<int> threads = singleThreaded ? QList<int>() << 1 : QList<int>() << 1 << 2 << 4 << 8 << 16 << 32 << 64 << 128;
    const QList<int> ms = QList<int>() << 1 << 10 << 100 << 1000 << 10000 << 100000;
    Q_FOREACH(int m, ms) {
        Q_FOREACH(int t, threads) {
            const QString name = tr("%1 threads, %2 values").arg(t).arg(m);
            // newRow expects const char*, but then qstrdup's it in the QTestData constructor. Eeeew.
            QTest::newRow(qPrintable(name)) << m << 256 << 256 << t;
        }
    }
}

QTEST_MAIN(QueueBenchmarksTest)

#include "QueueBenchmarks.moc"

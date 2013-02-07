#ifndef DELETETEST_H
#define DELETETEST_H

#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtTest/QtTest>

namespace ThreadWeaver { class Job; }

using namespace ThreadWeaver;

class DeleteTest : public QObject
{
    Q_OBJECT
public:
    DeleteTest();

private Q_SLOTS:
    void DeleteSequenceTest();

public Q_SLOTS: // not a test!
    void deleteSequence(ThreadWeaver::Job* job);

private:
    int m_finishCount;
    mutable QMutex m_finishMutex;
};

#endif

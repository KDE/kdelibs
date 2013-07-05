#ifndef DELETETEST_H
#define DELETETEST_H

#include <QtCore/QObject>
#include <QtTest/QtTest>
#include <QAtomicInt>

#include <Weaver/JobPointer.h>

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
    void deleteSequence(ThreadWeaver::JobPointer job);

Q_SIGNALS:
    void deleteSequenceTestCompleted();

private:
    QAtomicInt m_finishCount;
};

#endif

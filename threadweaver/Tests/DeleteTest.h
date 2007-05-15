#ifndef DELETETEST_H
#define DELETETEST_H

#include <QtCore/QCoreApplication>
#include <QtCore/QMutex>

namespace ThreadWeaver { class Job; }

using namespace ThreadWeaver;

class DeleteTest : public QCoreApplication
{
  Q_OBJECT

public:
  DeleteTest(int argc, char **argv);

public Q_SLOTS:
  void deleteSequence(ThreadWeaver::Job* job);

private:
  int m_finishCount;
  mutable QMutex m_finishMutex;
};

#endif

#ifndef DELETETEST_H
#define DELETETEST_H

#include <QtCore/QCoreApplication>

namespace ThreadWeaver { class Job; }

using namespace ThreadWeaver;

class DeleteTest : public QCoreApplication
{
  Q_OBJECT

public:
  DeleteTest(int argc, char **argv);

public slots:
  void deleteSequence(Job* job);
};

#endif

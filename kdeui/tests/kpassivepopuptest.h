#ifndef KPASSIVEPOPUPTEST_H
#define KPASSIVEPOPUPTEST_H

#include <qobject.h>

class Test : public QObject
{
  Q_OBJECT

public:
  Test() : QObject() {};
  ~Test() {};

public Q_SLOTS:
  void showIt();
  void showIt2();
  void showIt3();
  void showIt4();
  void showIt5();
};


#endif // KPASSIVEPOPUPTEST_H



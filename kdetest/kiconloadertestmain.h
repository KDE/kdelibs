// -*- C++ -*-

#ifndef DEMO
#define DEMO

#include <qwidget.h>

class QPushButton;

class MyMain : public QWidget
{
  Q_OBJECT
public:
  MyMain ();
  ~MyMain () {}

public slots:
  void changePix();

protected:
  QPushButton *test;
};

#endif

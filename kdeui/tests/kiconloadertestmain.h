// -*- C++ -*-

#ifndef DEMO
#define DEMO

#include <qwidget.h>

#include <kiconloaderdialog.h>

class QPushButton;
class QLineEdit;

class MyMain : public QWidget
{
  Q_OBJECT
public:
  MyMain ();
  ~MyMain () {delete icon_sel; }

public slots:
  void changePix();
  void insertPath();

protected:
  QPushButton *test;
  QLineEdit   *le_dir;
  KIconLoaderDialog *icon_sel;
};

#endif

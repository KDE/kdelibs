// $Id$

#ifndef KLOCALETEST_H
#define KLOCALETEST_H

#include <qwidget.h>

class QLabel;

/** test: a small test program for KLocale
    */
class Test : public QWidget
{
  Q_OBJECT;
  
public:
  /**@name methods */
  //@{
  /** Constructor  
	*/
  Test( QWidget *parent=0, const char *name=0 );
  /** Destructor
	*/
  ~Test();

private:
  QString showLocale(QString cat);
  void createFields();

  QLabel *label;
};
#endif // TEST_H

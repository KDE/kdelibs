// $Id$

#ifndef TEST_H
#define TEST_H

#include <qwidget.h>
#include <qlabel.h>
#include <kintegerline.h>

/** test: a small test program for KmwIntegerLine.
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
  void createFields();

  QLabel         *m_plb;
  QLabel         *m_plb1;
  QLabel         *m_plb2;
  QLabel         *m_plb3;

  KIntegerLine *m_ple1;
  KIntegerLine *m_ple2;
  KIntegerLine *m_ple3;
};
#endif // TEST_H

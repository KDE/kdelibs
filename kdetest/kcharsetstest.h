// $Id$

#ifndef TEST_H
#define TEST_H

#include <qwidget.h>

class QLabel;
class QComboBox;
class QLineEdit;

/** test: a small test program for KCharsets.
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

protected slots:
  void convert();
  void convert(int) { convert(); }
  void changeDisplayCharset(int);

private:
  void createFields();

  QLabel *inputCharsetLabel;
  QComboBox *inputCharsetCombo;
  QLabel *inputLabel;
  QLineEdit *inputEdit;
  QLabel *outputCharsetLabel;
  QComboBox *outputCharsetCombo;
  QLabel *displayCharsetLabel;
  QComboBox *displayCharsetCombo;
  QLabel *outputLabel;
  QLineEdit *outputEdit;
};
#endif // TEST_H

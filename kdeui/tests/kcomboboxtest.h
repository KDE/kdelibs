#ifndef _KCOMBOBOXTEST_H
#define _KCOMBOBOXTEST_H

#include <qwidget.h>
#include <qguardedptr.h>

class QTimer;
class QLineEdit;
class QPushButton;

class KComboBox;

class KComboBoxTest : public QWidget
{
  Q_OBJECT
    
public:
  KComboBoxTest ( QWidget *parent=0, const char *name=0 );
  ~KComboBoxTest();

private slots:
  void quitApp();
  void slotTimeout();
  void slotDisable();
  void slotReturnPressed();
  void slotActivated( int );
  void slotActivated( const QString& );
   
protected:
  KComboBox* m_ro;
  KComboBox* m_rw;
  KComboBox* m_hc;
  KComboBox* m_konqc;

  QPushButton* m_btnExit;
  QPushButton* m_btnEnable;

  QTimer* m_timer;
};

#endif

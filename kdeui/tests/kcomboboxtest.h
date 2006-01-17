#ifndef _KCOMBOBOXTEST_H
#define _KCOMBOBOXTEST_H

#include <qwidget.h>

class QTimer;
class QComboBox;
class QPushButton;

class KComboBox;

class KComboBoxTest : public QWidget
{
  Q_OBJECT

public:
  KComboBoxTest ( QWidget *parent=0);
  ~KComboBoxTest();

private slots:
  void quitApp();
  void slotTimeout();
  void slotDisable();
  void slotReturnPressed();
  void slotReturnPressed(const QString&);
  void slotActivated( int );
  void slotActivated( const QString& );

protected:
  QComboBox* m_qc;

  KComboBox* m_ro;
  KComboBox* m_rw;
  KComboBox* m_hc;
  KComboBox* m_konqc;


  QPushButton* m_btnExit;
  QPushButton* m_btnEnable;

  QTimer* m_timer;
};

#endif

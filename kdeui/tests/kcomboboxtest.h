#ifndef _KCOMBOBOXTEST_H
#define _KCOMBOBOXTEST_H

#include <qwidget.h>
#include <qguardedptr.h>

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
   void slotActivated( int );
   void slotReturnPressed ();
   void slotActivated( const QString& );
   
protected:
   QGuardedPtr<KComboBox> m_combobox;
   
   QPushButton* m_btnExit;
   QPushButton* m_btnReadOnly;
   QPushButton* m_btnEnable;
};

#endif

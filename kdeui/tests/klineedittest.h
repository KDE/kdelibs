#ifndef _KLINEEDITTEST_H
#define _KLINEEDITTEST_H

#include <qwidget.h>
#include <qguardedptr.h>

class QString;
class QPushButton;

class KLineEdit;

class KLineEditTest : public QWidget
{
    Q_OBJECT

public:
   KLineEditTest( QWidget *parent=0, const char *name=0 );
   ~KLineEditTest();
   KLineEdit* lineEdit() const { return m_lineedit; }

public slots:
   virtual void show ();
   
private slots:
   void quitApp();
   void slotHide();   
   void slotEnable( bool );
   void slotReadOnly( bool );   
   void slotReturnPressed();
   void resultOutput( const QString& );   
   void slotReturnPressed( const QString& );
   
protected:
   QGuardedPtr<KLineEdit> m_lineedit;
   QPushButton* m_btnExit;
   QPushButton* m_btnReadOnly;
   QPushButton* m_btnEnable;
   QPushButton* m_btnHide;
};

#endif

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
   KLineEditTest ( QWidget *parent=0, const char *name=0 );
   ~KLineEditTest();
   KLineEdit* lineEdit() const { return m_lineedit; }

private slots:
   void quitApp();
   void readOnly (bool);
   void setEnable (bool);
   void slotReturnPressed();
   void slotReturnPressed(const QString&);
   void resultOutput( const QString& );

protected:
   QGuardedPtr<KLineEdit> m_lineedit;
   QPushButton* m_btnExit;
   QPushButton* m_btnReadOnly;
   QPushButton* m_btnEnable;
};

#endif

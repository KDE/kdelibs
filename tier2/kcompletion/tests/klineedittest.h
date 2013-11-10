#ifndef _KLINEEDITTEST_H
#define _KLINEEDITTEST_H

#include <QWidget>
#include <QtCore/QPointer>

class QLabel;
class QLineEdit;
class QString;
class QPushButton;

class KLineEdit;

class KLineEditTest : public QWidget
{
    Q_OBJECT

public:
   KLineEditTest( QWidget *parent=0 );
   ~KLineEditTest();
   KLineEdit* lineEdit() const { return m_lineedit; }

public Q_SLOTS:
   virtual void show ();

private Q_SLOTS:
   void quitApp();
   void slotHide();
   void slotEnable( bool );
   void slotReadOnly( bool );
   void slotPassword( bool );
   void slotReturnPressed();
   void resultOutput( const QString& );
   void slotReturnPressed( const QString& );
   void slotPlaceholderText(bool click);
   void slotInvalidChar(int);
   void slotSetStyleSheet();

protected:
   KLineEdit* m_lineedit;
   QLineEdit* m_restrictedLine;
   QLabel* m_invalidCharLabel;
   QPushButton* m_btnExit;
   QPushButton* m_btnReadOnly;
   QPushButton* m_btnPassword;
   QPushButton* m_btnEnable;
   QPushButton* m_btnHide;
   QPushButton* m_btnPlaceholderText;
   QPushButton* m_btnClickMessage;
};

#endif

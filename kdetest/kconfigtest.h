#ifndef _KCONFIG_TEST_H
#define _KCONFIG_TEST_H

#include <kconfig.h>

//
// KConfigTestView contains lots of Qt widgets.
//

class KConfigTestView : public QDialog
{
  Q_OBJECT
public:
  KConfigTestView( QWidget *parent=0, const char *name=0 );
  ~KConfigTestView();

private slots:
  void appConfigEditReturnPressed();
  void groupEditReturnPressed();
  void keyEditReturnPressed();
  void writeButtonClicked();

private:
  QLabel* pAppFileLabel;
  QLineEdit* pAppFileEdit;
  QLabel* pGroupLabel;
  QLineEdit* pGroupEdit;
  QLineEdit* pKeyEdit;
  QLabel* pEqualsLabel;
  QLineEdit* pValueEdit;
  QPushButton* pWriteButton;
  QLabel* pInfoLabel1, *pInfoLabel2;
  QPushButton* pQuitButton;

  KConfig* pConfig;
  QFile* pFile;
  QTextStream* pStream;
};

#endif

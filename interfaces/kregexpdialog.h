#ifndef __kregexpdialog_h
#define __kregexpdialog_h
#include <kdialogbase.h>
#include <kapp.h>
class QLineEdit;
class KRegExpEditor;


class KRegExpDialog :public KDialogBase
{
Q_OBJECT

public:
  KRegExpDialog( QWidget* parent, const char* name = 0, bool modal = true, const QString& caption = QString::null );
  QString regexp() const;
  static bool isGuiEditor();
  
public slots:
  void slotSetRegExp( const QString& regexp );

protected slots:
  void slotHelp();

private:
  QLineEdit* _lineEditor;
  KRegExpEditor* _guiEditor;
};

#endif // __kregexpdialog_h

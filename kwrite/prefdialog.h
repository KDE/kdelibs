#ifndef __prefdialog_h__
#define __prefdialog_h__

#include <kdialogbase.h>

class IndentConfigTab;
class SelectConfigTab;
class EditConfigTab;
class KSpellConfig;
class ColorConfig;
class KWrite;

class PreferencesDlg : public KDialogBase
{
  Q_OBJECT

public:

  PreferencesDlg( KWrite*, QWidget* parent = 0, char* name = 0 );
  ~PreferencesDlg();

  void applySettings();

  static void doSettings( KWrite* );

protected slots:

  virtual void slotOk();
  virtual void slotHelp();
  virtual void slotApply();
  virtual void slotDefault();

private:

  IndentConfigTab *m_indentConfig;
  SelectConfigTab *m_selectConfig;
  EditConfigTab   *m_editConfig;
  KSpellConfig    *m_ksc;
  KWrite          *m_kwrite;
  ColorConfig     *m_colorConfig;
};

#endif // __prefdialog_h__

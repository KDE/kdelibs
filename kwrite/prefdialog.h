#ifndef __prefdialog_h__
#define __prefdialog_h__

#include <kdialogbase.h>

#include "kwrite_keys.h"

class KWrite;
class IndentConfigTab;
class SelectConfigTab;
class EditConfigTab;
class ViewConfigTab;
class ColorConfig;
class KSpellConfig;

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

protected:

  KWrite          *m_kwrite;
  IndentConfigTab *m_indentConfig;
  SelectConfigTab *m_selectConfig;
  EditConfigTab   *m_editConfig;
  ViewConfigTab   *m_viewConfig;
  ColorConfig     *m_colorConfig;
  KWKeyData        m_keyData;
  KWKeyConfigTab  *m_keyConfig;
  KSpellConfig    *m_ksc;
};

#endif // __prefdialog_h__

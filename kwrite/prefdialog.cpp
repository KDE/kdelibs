#include <qvbox.h>

#include <klocale.h>
#include <ksconfig.h>
#include <kiconloader.h>
#include <kkeydialog.h>

#include <kwrite/kwrite_doc.h>
#include <kwrite/kwrite_view.h>

#include "kwdialog.h"
#include "prefdialog.h"

PreferencesDlg::PreferencesDlg( KWrite* kwrite, QWidget* parent, char* name )
  : KDialogBase( IconList, "Configure KWrite", Ok | Cancel | Default | Help, Ok, parent, name, true, true )
  , m_kwrite( kwrite )
{
  QVBox *vbox;
  QColor colors[ 5 ];

  // indent options
  vbox = addVBoxPage( i18n( "Indent" ), QString::null, UserIcon( "misc" ) );
  m_indentConfig = new IndentConfigTab( vbox, kwrite );

  // select options
  vbox = addVBoxPage( i18n( "Select" ), QString::null, UserIcon( "misc" ) );
  m_selectConfig = new SelectConfigTab( vbox, kwrite );

  // edit options
  vbox = addVBoxPage( i18n( "Edit" ), QString::null, UserIcon( "misc" ) );
  m_editConfig = new EditConfigTab( vbox, kwrite );

  // color options
  vbox = addVBoxPage( i18n( "Colors" ), QString::null, UserIcon( "colors" ) );
  m_colorConfig = new ColorConfig( vbox );
  //kwrite->getColors( colors );
  //m_colorConfig->setColors( colors );

  // highlighting options
  vbox = addVBoxPage( i18n( "Highlighting" ), QString::null, UserIcon( "misc" ) );
  (void) new QWidget( vbox );

  // key binding options
  vbox = addVBoxPage( i18n( "Key bindings"), QString::null, UserIcon( "misc" ) );
  (void) new QWidget( vbox );
  //QDict<KKeyEntry> *keys;
  //(void) new KKeyChooser( keys, vbox );
  //KGuiCmdConfigTab *keys = new KGuiCmdConfigTab( vbox , &cmdMngr );
  //qtd->addTab(keys, i18n("Keys"));

  // spell checker
  vbox = addVBoxPage( i18n( "Spellchecker" ), QString::null, BarIcon( "spellcheck" ) );
  m_ksc = new KSpellConfig( vbox, 0L, kwrite->ksConfig(), false );
}

PreferencesDlg::~PreferencesDlg()
{
}

void PreferencesDlg::applySettings()
{
  // indent options
  m_indentConfig->getData( m_kwrite );

  // select options
  m_selectConfig->getData( m_kwrite );

  // select options

  // edit options
  m_editConfig->getData(m_kwrite);

  // select options

  // color options
  //m_colorConfig->getColors( m_kwrite->doc()->colors() );

  // select options

  // spell checker
  m_ksc->writeGlobalSettings();
  m_kwrite->setKSConfig(*m_ksc);

  // select options
  /*
  keys
      cmdMngr.changeAccels();
      cmdMngr.writeConfig(kapp->config());
    } else {
      // cancel keys
      cmdMngr.restoreAccels();
  */
}

void PreferencesDlg::doSettings( KWrite* kwrite )
{
  PreferencesDlg *dlg = new PreferencesDlg( kwrite );

  dlg->exec();

  delete dlg;
}

void PreferencesDlg::slotOk()
{
  applySettings();
  accept();
}

void PreferencesDlg::slotHelp()
{
    // KSpellConfig has its own help pages
    if( activePageIndex() == 6 )
	m_ksc->activateHelp();
    else
	KDialogBase::slotHelp();
}

void PreferencesDlg::slotApply()
{
    applySettings();
}

void PreferencesDlg::slotDefault()
{
}

/*
    $Id$

    Copyright (C) 2000 Michael Koch <koch@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qdir.h>
#include <qvbox.h>
#include <qfile.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qsplitter.h>
#include <qcheckbox.h>
#include <qtextstream.h>

#include <kapp.h>
#include <kdebug.h>
#include <kaction.h>
#include <klocale.h>
#include <kstddirs.h>
#include <khelpmenu.h>
#include <kstdaction.h>
#include <kbugreport.h>
#include <kstatusbar.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kcmdlineargs.h>
#include <kedittoolbar.h>

#include <kwrite/kwrite_part.h>
#include <kwrite/kwrite_shell.h>

#define ID_STATUSBAR_INFO      0
#define ID_STATUSBAR_CURSOR    1
#define ID_STATUSBAR_NUMLOCK   2
#define ID_STATUSBAR_CAPSLOCK  3
#define ID_STATUSBAR_MODE      4

KWriteShell::KWriteShell( const KURL& url )
  : KParts::MainWindow( "mainwindow" )
{
  setXMLFile( "kwrite_shell.rc" );

  m_part = new KWritePart( this, this );
  m_part->setReadWrite(); // read-write mode

  connect( m_part, SIGNAL( newCursorPos( const QPoint& ) ), this, SLOT( newCursorPos( const QPoint& ) ) );

  setupActions();

  setView( m_part->editorWidget() );
  m_part->editorWidget()->setMinimumSize( 300, 200 );

  show();
  createGUI( m_part );

/*
  KActionCollection *a = actionCollection();
  for (uint z = 0; z < a->count(); z++) {
    printf("Action: %s\n", a->action(z)->text().ascii());
  }*/

  m_pStatusBar = new KStatusBar( this, "statusbar" );
  setStatusBar( m_pStatusBar );

  m_pStatusBar->insertItem( "TestText", ID_STATUSBAR_INFO, 1 );
  m_pStatusBar->insertItem( "22:12", ID_STATUSBAR_CURSOR, 0, true );
  m_pStatusBar->insertItem( "INS/OVR", ID_STATUSBAR_MODE, 0, true );
  m_pStatusBar->insertItem( "NUM", ID_STATUSBAR_NUMLOCK, 0, true );
  m_pStatusBar->insertItem( "CAPS", ID_STATUSBAR_CAPSLOCK, 0, true );

  m_pStatusBar->show();

  m_eolAction->setCurrentItem( 0 );

  restoreOptions();

  if( !url.isEmpty() )
    openURL( url );

  setCaption( url.url() );

  // how do we set the focus at startup?
  // m_part->editorWidget()->setFocus();
}

KWriteShell::~KWriteShell()
{
  saveOptions();
}

void KWriteShell::setupActions()
{
  
  m_helpMenu = new KHelpMenu( this );

  KStdAction::openNew( this, SLOT( slotFileNew() ), actionCollection(), "file_new" );
  KStdAction::open( this, SLOT( slotFileOpen() ), actionCollection(), "file_open" );
  m_recent = KStdAction::openRecent( this, SLOT(openURL(const KURL&)), actionCollection() );
  KStdAction::save( this, SLOT( slotFileSave() ), actionCollection(), "file_save" );
  KStdAction::saveAs( this, SLOT( slotFileSaveAs() ), actionCollection(), "file_save_as" );
  KStdAction::revert( this, SLOT( slotFileRevert() ), actionCollection(), "file_revert" );
  KStdAction::print( this, SLOT( slotFilePrint() ), actionCollection(), "file_print" );
  (new KAction( i18n( "New view" ), 0, this, SLOT( slotFileNewView() ), actionCollection(), "file_new_view" ))->setEnabled(false);
  new KAction( i18n( "New window" ), 0, this, SLOT( slotFileNewWindow() ), actionCollection(), "file_new_window" );
  KStdAction::close( this, SLOT( close() ), actionCollection(), "shell_close" );
  KStdAction::quit( this, SLOT( close() ), actionCollection(), "shell_quit" );

  QStringList eolList;

  eolList.append( "Unix" );
  eolList.append( "Macintosh" );
  eolList.append( "Windows/DOS" );

  m_eolAction = new KSelectAction( i18n("End of line"), 0, actionCollection(), "end_of_line" );
  m_eolAction->setItems( eolList );
  m_eolAction->setCurrentItem( 1 );
  connect( m_eolAction, SIGNAL( activated( int ) ), m_part, SLOT( slotEOL( int ) ) );

  KStdAction::preferences( m_part, SLOT( slotOptions() ), actionCollection(), "configure" );
  KStdAction::configureToolbars( this, SLOT( slotConfigureToolbars() ), actionCollection(), "configure_toolbars" );

  m_vertical = new KToggleAction( i18n( "Vertical selections" ), 0, m_part, SLOT( slotVerticalSelections() ),
				  actionCollection(), "vertical_selections" );
  m_toolbar = KStdAction::showToolbar( this, SLOT( slotShowToolbar() ), actionCollection(), "show_toolbar" );
  m_statusbar = KStdAction::showStatusbar( this, SLOT( slotShowStatusbar() ), actionCollection(), "show_statusbar" );
  m_path = new KToggleAction( i18n( "Show &Path" ), 0, this, SLOT( slotShowPath() ), actionCollection(), "show_path" );
  KStdAction::saveOptions( this, SLOT( saveOptions() ), actionCollection(), "save_options" );

  KStdAction::helpContents( m_helpMenu, SLOT( appHelpActivated() ), actionCollection(), "contents" );
  KStdAction::whatsThis( m_helpMenu, SLOT( contextHelpActivated() ), actionCollection(), "whats_this" );
  KStdAction::aboutApp( this, SLOT( slotHelpAboutKWrite() ), actionCollection(), "about_kwrite" );
  KStdAction::aboutKDE( m_helpMenu, SLOT( aboutKDE() ), actionCollection(), "about_kde" );
  KStdAction::reportBug( m_helpMenu, SLOT( reportBug() ), actionCollection(), "report_bug" );
}

void KWriteShell::openURL( const KURL& url )
{
    m_recent->addURL( url );
    m_part->openURL( url );
}

void KWriteShell::slotFileNew()
{
  setCaption();
  m_part->newDoc();
}

void KWriteShell::slotFileOpen()
{
  KURL url = KFileDialog::getOpenURL( QString::null, QString::null, 0L, "file dialog" );

  if( !url.isEmpty() )
  {
    setCaption( url.url() );
    openURL( url );
  }
}

void KWriteShell::slotFileOpenRecent( int /*index*/ )
{
  // TODO: realize this
}

void KWriteShell::slotFileSave()
{
  if( m_part->isModified() )
  {
    if( !m_part->url().isEmpty() &&
         m_part->isReadWrite() )
      m_part->saveAs( m_part->url() );
    else
      slotFileSaveAs();
  }
  else
    emit statusMsg( i18n( "No changes need to be saved" ) );
}

void KWriteShell::slotFileSaveAs()
{
  KURL url = KFileDialog::getSaveURL( QString::null, QString::null, 0L, "file dialog" );

  if( !url.isEmpty() )
  {
    setCaption( url.url() );
    m_recent->addURL( url );
    m_part->saveAs( url );
  }
}

void KWriteShell::slotFileRevert()
{
  KURL url = m_part->url();

  if( !url.isEmpty() )
    openURL( url );
  else
    slotFileNew();
}

void KWriteShell::slotFilePrint()
{
    m_part->slotPrint();
}

void KWriteShell::slotFileNewView()
{
}

void KWriteShell::slotFileNewWindow()
{
  KWriteShell * shell = new KWriteShell;
  shell->show();
}

void KWriteShell::slotShowToolbar()
{
  if( toolBar()->isVisible () )
    toolBar()->hide();
  else
    toolBar()->show();
}

void KWriteShell::slotShowStatusbar()
{
  if( statusBar()->isVisible () )
    statusBar()->hide();
  else
    statusBar()->show();
}

void KWriteShell::slotShowPath()
{
  setCaption( m_part->url().url() );
}

void KWriteShell::slotConfigureToolbars()
{
  KEditToolbar edit(factory());
  edit.exec();
}

void KWriteShell::saveOptions()
{
  KConfig* config = KGlobal::config();

  // set group
  config->setGroup( "Appearance" );

  config->writeEntry( "ShowToolbar", m_toolbar->isChecked() );
  config->writeEntry( "ShowStatusbar", m_statusbar->isChecked() );
  config->writeEntry( "ShowPath", m_path->isChecked() );

  config->writeEntry( "Width", width() );
  config->writeEntry( "Height", height() );

  m_recent->saveEntries( config );

  m_part->saveConfig( config );

  config->sync();
}

void KWriteShell::restoreOptions()
{
  KConfig* config = KGlobal::config();

  // set group
  config->setGroup( "Appearance" );

  bool showToolbar = config->readBoolEntry( "ShowToolbar", true );

  m_toolbar->blockSignals( true );
  m_toolbar->setChecked( showToolbar );
  m_toolbar->blockSignals( false );

  if( showToolbar )
    toolBar()->show();
  else
    toolBar()->hide();

  bool showStatusbar = config->readBoolEntry( "ShowStatusbar", true );

  m_statusbar->blockSignals( true );
  m_statusbar->setChecked( showStatusbar );
  m_statusbar->blockSignals( false );

  if( showStatusbar )
    statusBar()->show();
  else
    statusBar()->hide();

  bool showPath = config->readBoolEntry( "ShowPath", true );

  m_path->blockSignals( true );
  m_path->setChecked( showPath );
  m_path->blockSignals( false );

  int width = config->readNumEntry( "Width", 550 );
  int height = config->readNumEntry( "Height", 400 );

  resize( width, height );

  m_recent->loadEntries( config );

  m_part->restoreConfig( config );
}

void KWriteShell::slotHelpAboutKWrite()
{
  KMessageBox::about( 0, i18n(
"KWrite Version " KWRITE_VERSION "\n\n"
"Author:\n"
"Jochen Wilhelmy <digisnap@cs.tu-berlin.de>\n\n"
"Co-authors:\n"
"Glen Parker <glenebob@nwlink.com>\n"
"Michael Koch <koch@kde.org>\n"
  ));
}

void KWriteShell::newCursorPos( const QPoint& cursor )
{
  // TODO: change entry in statusbar
}

void KWriteShell::setCaption( const QString& caption )
{
  QString tmp;

  if( caption.isEmpty() )
    tmp = i18n( "unknown" );
  else
    tmp = caption;

  if( !m_path->isChecked() )
    tmp = tmp.right( tmp.length() - tmp.findRev( '/' ) - 1 );

  KParts::MainWindow::setCaption( tmp );
}

void KWriteShell::statusMsg( const QString& text )
{
  //m_pStatusBar->changeItem( text, ID_STATUSBAR_INFO );
  m_pStatusBar->message( text, 5000 );
}

bool KWriteShell::queryClose()
{
  return m_part->closeURL();
}


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
#include <qfile.h>
#include <qsplitter.h>
#include <qtextstream.h>

#include <kapp.h>
#include <kxmlgui.h>
#include <kaction.h>
#include <kconfig.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kstdaction.h>
#include <kmessagebox.h>
#include <kfiledialog.h>

#include <kparts/partmanager.h>
#include <kparts/mainwindow.h>

#include <kwrite/highlight.h>
#include <kwrite/kwrite_doc.h>
#include <kwrite/kwrite_view.h>
#include <kwrite/kwrite_part.h>
#include <kwrite/kwrite_factory.h>

#include "kwdialog.h"
#include "ktextprint.h"
#include "prefdialog.h"

KWritePart::KWritePart( QWidget * parentWidget, QObject *parent, bool bBrowser )
 : KParts::ReadWritePart( parent, "KWritePart" )
{
  setInstance( KWriteFactory::instance() );

  if ( bBrowser )
  {
    setXMLFile( "kwrite_browser.rc" );
    (void)new KWriteBrowserExtension( this );
  }
  else
    setXMLFile( "kwrite.rc" );

  m_doc = new KWriteDoc( HlManager::self() );
  m_kwrite = new KWrite( m_doc, parentWidget );

  // check this: at startup the edit widget does not seem to have the focus
  m_kwrite->setFocus();
  m_kwrite->setFocusPolicy( QWidget::ClickFocus );
  setWidget( m_kwrite );

  m_cut = KStdAction::cut( this, SLOT( slotCut() ), actionCollection(), "edit_cut" );
  m_copy = KStdAction::copy( this, SLOT( slotCopy() ), actionCollection(), "edit_copy" );
  m_paste = KStdAction::paste( this, SLOT( slotPaste() ), actionCollection(), "edit_paste" );
  m_undo = KStdAction::undo( this, SLOT( slotUndo() ), actionCollection(), "edit_undo" );
  m_redo = KStdAction::redo( this, SLOT( slotRedo() ), actionCollection(), "edit_redo" );
  (void)new KAction( i18n( "Undo/redo history" ), 0, this, SLOT( slotUndoRedoHistory() ), actionCollection(), "undoredohistory" );
  m_indent = new KAction( i18n( "&Indent"), 0, this, SLOT( slotIndent() ), actionCollection(), "indent" );
  m_unindent = new KAction( i18n( "Unindent" ), 0, this, SLOT( slotUnindent() ), actionCollection(), "unindent" );
  m_cleanIndent = new KAction( i18n( "Clean indent" ), 0, this, SLOT( slotCleanIndent() ), actionCollection(), "clean_indent" );
  KStdAction::selectAll( this, SLOT( slotSelectAll() ), actionCollection(), "select_all" );
  (void)new KAction( i18n( "Unselect all" ), 0, this, SLOT( slotDeselectAll() ), actionCollection(), "unselect_all" );
  (void)new KAction( i18n( "Invert selection" ), 0, this, SLOT( slotInvertSelection() ), actionCollection(), "invert_select" );
  m_spell = KStdAction::spelling( this, SLOT( slotSpellChecking() ), actionCollection(), "spell_checking" );

  // TODO: get this list from HlManager
  QStringList langList;

  langList.append( "Normal" );
  langList.append( "C" );
  langList.append( "C++" );
  langList.append( "Objective-C" );
  langList.append( "Java" );
  langList.append( "HTML" );
  langList.append( "Bash" );
  langList.append( "Modula 2" );
  langList.append( "Ada" );
  langList.append( "Python" );
  langList.append( "Perl" );
  langList.append( "Sather" );
  langList.append( "Latex" );
  langList.append( "IDL" );

  m_langAction = new KSelectAction( i18n( "Set highlighting" ), 0, actionCollection(), "highlight_select" );
  m_langAction->setItems( langList );
  m_langAction->setCurrentItem( 1 );
  connect( m_langAction, SIGNAL( activated( int ) ), this, SLOT( slotHighlighting( int ) ) );

  KStdAction::find( this, SLOT( slotFind() ), actionCollection(), "find" );
  m_replace = KStdAction::replace( this, SLOT( slotReplace() ), actionCollection(), "replace" );
  KStdAction::findNext( this, SLOT( slotFindAgain() ), actionCollection(), "find_again" );
  KStdAction::gotoLine( this, SLOT( slotGotoLine() ), actionCollection(), "goto_line" );

  (void)new KAction( i18n( "Set bookmark" ), KStdAccel::addBookmark(), this, SLOT( slotSetBookmark() ), actionCollection(), "set_bookmark" );
  KStdAction::addBookmark( this, SLOT( slotAddBookmark() ), actionCollection(), "add_bookmark" );
  (void)new KAction( i18n( "Clear bookmarks" ), 0, this, SLOT( slotClearBookmarks() ), actionCollection(), "clear_bookmarks" );

  // TODO: add KListAction here for bookmark list.

  connect( m_kwrite, SIGNAL( statusMsg( const QString& ) ), this, SIGNAL( setStatusBarText( const QString& ) ) );
  connect( m_kwrite, SIGNAL( newCurPos() ), this, SLOT( newCurPos() ) );
  connect( m_kwrite, SIGNAL( newStatus() ), this, SLOT( newStatus() ) );
  connect( m_kwrite, SIGNAL( fileChanged() ), this, SLOT( newCaption() ) );
  connect( m_kwrite, SIGNAL( newUndo() ),this,SLOT( newUndo() ) );
  //connect( m_kwrite->view(), SIGNAL( dropEventPass( QDropEvent* ) ), this, SLOT( slotDropEvent( QDropEvent* ) ) );

  m_cut->setEnabled( false );
  m_copy->setEnabled( false );
  m_paste->setEnabled( false );
  m_undo->setEnabled( false );
  m_redo->setEnabled( false );

  m_langAction->setCurrentItem( 0 );
}

KWritePart::~KWritePart()
{
}

QWidget* KWritePart::editorWidget()
{
    return m_kwrite;
}

void KWritePart::setReadWrite( bool rw )
{
    m_doc->setReadOnly( !rw );
    KParts::ReadWritePart::setReadWrite( rw );
}

bool KWritePart::openFile()
{
    m_kwrite->loadURL( m_file );
    m_kwrite->doc()->setFileName( m_file );

    return true;
}

bool KWritePart::saveFile()
{
  QFile f( m_file );
  QString s;

  if( f.open( IO_WriteOnly ) )
  {
      QTextStream t( &f );
      t << m_kwrite->text();
      f.close();
      m_kwrite->setModified( false );

      return true;
  }
  else
      return false;
}

void KWritePart::newDoc()
{
    setModified( false );
    m_url = KURL();
    m_file = "";
    m_kwrite->newDoc();
}

void KWritePart::slotPrint()
{
    QString title = url().filename();

    KTextPrintConfig::print( m_kwrite, KGlobal::config(), true,
			     title, m_doc->numLines(),
			     this, SLOT( doPrint( KTextPrint& ) ) );
}

void KWritePart::slotCut()
{
    m_kwrite->cut();
}

void KWritePart::slotCopy()
{
    m_kwrite->copy();
}

void KWritePart::slotPaste()
{
    m_kwrite->paste();
}

void KWritePart::slotUndo()
{
    m_kwrite->undo();
}

void KWritePart::slotRedo()
{
  m_kwrite->redo();
}

void KWritePart::slotUndoRedoHistory()
{
  m_kwrite->undoHistory();
}

void KWritePart::slotIndent()
{
  debug( "not supported at the moment" ); // TODO:
  //m_kwrite->ident();
}

void KWritePart::slotUnindent()
{
  debug( "not supported at the moment" ); // TODO:
  //m_kwrite->unIdent();
}

void KWritePart::slotCleanIndent()
{
    debug( "not supported at the moment" ); // TODO:
    //m_kwrite->cleanIdent();
}

void KWritePart::slotSelectAll()
{
    m_kwrite->selectAll();
}

void KWritePart::slotDeselectAll()
{
    m_kwrite->deselectAll();
}

void KWritePart::slotInvertSelection()
{	
    debug( "not supported at the moment" ); // TODO:
    //m_kwrite->invertselection();
}

void KWritePart::slotSpellChecking()
{
    m_kwrite->spellcheck();
}

void KWritePart::slotFind()
{
    m_kwrite->find();
}

void KWritePart::slotReplace()
{
    m_kwrite->replace();
}

void KWritePart::slotFindAgain()
{
    m_kwrite->findAgain();
}

void KWritePart::slotGotoLine()
{
    m_kwrite->gotoLine();
}

void KWritePart::slotSetBookmark()
{
    m_kwrite->setBookmark();
}

void KWritePart::slotAddBookmark()
{
    m_kwrite->addBookmark();
}

void KWritePart::slotClearBookmarks()
{
    m_kwrite->clearBookmarks();
}

void KWritePart::slotHighlighting( int language )
{
    m_kwrite->setHl( language );
}

void KWritePart::slotOptions()
{
    PreferencesDlg::doSettings( m_kwrite );
}

void KWritePart::slotEOL( int )
{
    // TODO
}

void KWritePart::slotVerticalSelections()
{
    // TODO
}

void KWritePart::saveConfig( KConfig* /*config*/ )
{
    // TODO
}

void KWritePart::restoreConfig( KConfig* /*config*/ )
{
    // TODO
}

void KWritePart::newStatus()
{
    debug( "KWritePart::newStatus" );

    if( m_kwrite->isModified() )
	setModified();

    if( isReadWrite() )
    {
	// TODO : insert "R/O" in statusbar ( "INS"/"OVR"-field )
    }

    // TODO : write modified flag to statusbar

    m_cut->setEnabled( isReadWrite() );
    m_copy->setEnabled( isReadWrite() );
    m_paste->setEnabled( isReadWrite() );
    m_replace->setEnabled( isReadWrite() );
    m_indent->setEnabled( isReadWrite() );
    m_unindent->setEnabled( isReadWrite() );
    m_cleanIndent->setEnabled( isReadWrite() );
    m_spell->setEnabled( isReadWrite() );

    newUndo();
}

void KWritePart::newCurPos()
{
  emit newCursorPos( QPoint( m_kwrite->currentLine() + 1,
			     m_kwrite->currentColumn() + 1 ) );
}

void KWritePart::newCaption()
{
  debug( "KWritePart::newCaption" );

  // TODO : set caption
}

void KWritePart::newUndo()
{
  int state = m_kwrite->undoState();

  if( state & 1 )
    m_undo->setEnabled( true );
  else
    m_undo->setEnabled( false );

  if( state & 2 )
    m_redo->setEnabled( true );
  else
    m_redo->setEnabled( false );

/*
  QString t;
  int state, uType, rType;

  state = m_kwrite->undoState();
  edit->setItemEnabled( menuUndoHist, ( state & 1 || state & 2 ) );
  t = cmdMngr.getCommand( ctEditCommands, cmUndo )->getName();

  if( state & 1 )
  {
    uType = m_kwrite->nextUndoType();
    edit->setItemEnabled( menuUndo, true );
    toolBar()->setItemEnabled( toolUndo, true );
    t += ' ';
    t += i18n( m_kwrite->undoTypeName( uType ) );
  }
  else
  {
    edit->setItemEnabled( menuUndo, false );
    toolBar()->setItemEnabled( toolUndo, false );
  }

  edit->setText( t, menuUndo );
  t = cmdMngr.getCommand( ctEditCommands, cmRedo )->getName();

  if( state & 2 )
  {
    rType = m_kwrite->nextRedoType();
    edit->setItemEnabled( menuRedo, true );
    toolBar()->setItemEnabled( toolRedo, true );
    t += ' ';
    t += i18n( m_kwrite->undoTypeName( rType ) );
  }
  else
  {
    edit->setItemEnabled( menuRedo, false );
    toolBar()->setItemEnabled( toolRedo, false );
  }

  edit->setText( t, menuRedo );
*/
}

void KWritePart::doPrint( KTextPrint& text )
{
}

KWriteBrowserExtension::KWriteBrowserExtension( KWritePart *part )
 : KParts::BrowserExtension( part )
{
}

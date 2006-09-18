/* This file is part of the KDE libraries
   Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>
                 2005 Michael Brade <brade@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "ktextedit.h"

#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QScrollBar>
#include <QTextCursor>

//#include <ksyntaxhighlighter.h>
//#include <kspell.h>
#include <kcursor.h>
#include <kglobalsettings.h>
#include <kstdaccel.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdialog.h>
#include <QMenu>

class KSpell;

//TODO: Use kspell2 in the future. 
//Use ksyntaxhighlighter from kspell2


class KTextEdit::Private
{
  public:
    Private( KTextEdit *_parent )
      : parent( _parent ),
        customPalette( false ),
        checkSpellingEnabled( false )
//      highlighter( 0 ),
//      spell( 0 )
    {
    }

    ~Private()
    {
//      delete highlighter;
//      delete spell;
    }

    void slotSpellCheckReady( KSpell *s );
    void slotSpellCheckDone( const QString &s );
    void spellCheckerMisspelling( const QString &text, const QStringList &, unsigned int pos );
    void spellCheckerCorrected( const QString &, const QString &, unsigned int );
    void spellCheckerFinished();
    void toggleAutoSpellCheck();
    void slotAllowTab();
    void menuActivated( QAction* action );

    void posToRowCol( unsigned int pos, unsigned int &line, unsigned int &col );

    KTextEdit *parent;
    bool customPalette;
    QAction *autoSpellCheckAction;
    QAction *allowTab;
    QAction *spellCheckAction;

    bool checkSpellingEnabled;
#if 0
    KDictSpellingHighlighter *highlighter;
    KSpell *spell;
#endif
};

void KTextEdit::Private::slotSpellCheckReady( KSpell *s )
{
// this is for Zack...
//
//    s->check( text() ); // TODO: toPlainText()?? (MiB)
//    connect( s, SIGNAL( done( const QString & ) ), this, SLOT( slotSpellCheckDone( const QString & ) ) );
}

void KTextEdit::Private::slotSpellCheckDone( const QString &s )
{
//    if ( s != text() ) // TODO: toPlainText()?? (MiB)
//        setText( s ); // setPlainText() ?! we'd loose rich text info
}

void KTextEdit::Private::spellCheckerMisspelling( const QString &text, const QStringList &, unsigned int pos )
{
//  highlightWord( text.length(), pos );
}

void KTextEdit::Private::spellCheckerCorrected( const QString &oldWord, const QString &newWord, unsigned int pos )
{
// TODO
/*
  unsigned int l = 0;
  unsigned int cnt = 0;

  if ( oldWord != newWord ) {
    posToRowCol( pos, l, cnt );
    setSelection( l, cnt, l, cnt + oldWord.length() );
    removeSelectedText();
    insert( newWord );
  }
*/
}

void KTextEdit::Private::spellCheckerFinished()
{
//    delete d->spell;
//    d->spell = 0L;
}

void KTextEdit::Private::toggleAutoSpellCheck()
{
  parent->setCheckSpellingEnabled( !checkSpellingEnabled );
}

void KTextEdit::Private::slotAllowTab()
{
  parent->setTabChangesFocus( !parent->tabChangesFocus() );
}

void KTextEdit::Private::menuActivated( QAction* action )
{
  if ( action == spellCheckAction )
    parent->checkSpelling();
  else if ( action == autoSpellCheckAction )
    toggleAutoSpellCheck();
  else if ( action == allowTab )
    slotAllowTab();
}

void KTextEdit::Private::posToRowCol( unsigned int pos, unsigned int &line, unsigned int &col )
{
// TODO
/*
  for ( line = 0; line < static_cast<uint>( lines() ) && col <= pos; line++ )
    col += paragraphLength( line ) + 1;

  line--;
  col = pos - col + paragraphLength( line ) + 1;
*/
}



KTextEdit::KTextEdit( const QString& text, QWidget *parent )
  : QTextEdit( text, parent ), d( new Private( this ) )
{
  KCursor::setAutoHideCursor( this, true, false );
}

KTextEdit::KTextEdit( QWidget *parent )
  : QTextEdit( parent ), d( new Private( this ) )
{
  KCursor::setAutoHideCursor( this, true, false );
}

KTextEdit::~KTextEdit()
{
  delete d;
}

void KTextEdit::keyPressEvent( QKeyEvent *event )
{
  int key = event->key() | event->modifiers();

  if ( KStdAccel::copy().contains( key ) ) {
    copy();
    event->accept();
    return;
  } else if ( KStdAccel::paste().contains( key ) ) {
    paste();
    event->accept();
    return;
  } else if ( KStdAccel::cut().contains( key ) ) {
    cut();
    event->accept();
    return;
  } else if ( KStdAccel::undo().contains( key ) ) {
    document()->undo();
    event->accept();
    return;
  } else if ( KStdAccel::redo().contains( key ) ) {
    document()->redo();
    event->accept();
    return;
  } else if ( KStdAccel::deleteWordBack().contains( key ) ) {
    deleteWordBack();
    event->accept();
    return;
  } else if ( KStdAccel::deleteWordForward().contains( key ) ) {
    deleteWordForward();
    event->accept();
    return;
  } else if ( KStdAccel::backwardWord().contains( key ) ) {
    QTextCursor cursor = textCursor();
    cursor.movePosition( QTextCursor::PreviousWord );
    setTextCursor( cursor );
    event->accept();
    return;
  } else if ( KStdAccel::forwardWord().contains( key ) ) {
    QTextCursor cursor = textCursor();
    cursor.movePosition( QTextCursor::NextWord );
    setTextCursor( cursor );
    event->accept();
    return;
  } else if ( KStdAccel::next().contains( key ) ) {
    QTextCursor cursor = textCursor();
    int targetY = verticalScrollBar()->value() + viewport()->height();
    bool moved = false;
    do {
      moved = cursor.movePosition( QTextCursor::Down );
      setTextCursor( cursor );
    } while ( moved && verticalScrollBar()->value() < targetY );
    event->accept();
    return;
  } else if ( KStdAccel::prior().contains( key ) ) {
    QTextCursor cursor = textCursor();
    int targetY = verticalScrollBar()->value() - viewport()->height();
    bool moved = false;
    do {
      moved = cursor.movePosition( QTextCursor::Up );
      setTextCursor( cursor );
    } while ( moved && verticalScrollBar()->value() > targetY );
    event->accept();
    return;
  } else if ( KStdAccel::home().contains( key ) ) {
    QTextCursor cursor = textCursor();
    cursor.movePosition( QTextCursor::Start );
    setTextCursor( cursor );
    event->accept();
    return;
  } else if ( KStdAccel::end().contains( key ) ) {
    QTextCursor cursor = textCursor();
    cursor.movePosition( QTextCursor::End );
    setTextCursor( cursor );
    event->accept();
    return;
  } else if ( KStdAccel::beginningOfLine().contains( key ) ) {
    QTextCursor cursor = textCursor();
    cursor.movePosition( QTextCursor::StartOfLine );
    setTextCursor( cursor );
    event->accept();
    return;
  } else if ( KStdAccel::endOfLine().contains( key ) ) {
    QTextCursor cursor = textCursor();
    cursor.movePosition( QTextCursor::EndOfLine );
    setTextCursor( cursor );
    event->accept();
    return;
  } else if ( KStdAccel::pasteSelection().contains( key ) ) {
    QString text = QApplication::clipboard()->text( QClipboard::Selection );
    if ( !text.isEmpty() )
      insertPlainText( text );  // TODO: check if this is html? (MiB)
    event->accept();
    return;
  } else if ( event->modifiers() == Qt::ControlModifier &&
            (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) &&
              qobject_cast<KDialog*>( topLevelWidget() ) ) {
    // ignore Ctrl-Return so that KDialogs can close the dialog
    event->ignore();
    return;
  }

  QTextEdit::keyPressEvent( event );
}

void KTextEdit::deleteWordBack()
{
  QTextCursor cursor = textCursor();
  cursor.clearSelection();
  cursor.movePosition( QTextCursor::PreviousWord, QTextCursor::KeepAnchor );
  cursor.removeSelectedText();
}

void KTextEdit::deleteWordForward()
{
  QTextCursor cursor = textCursor();
  cursor.clearSelection();
  cursor.movePosition( QTextCursor::EndOfWord, QTextCursor::KeepAnchor );
  cursor.removeSelectedText();
}

void KTextEdit::contextMenuEvent( QContextMenuEvent *event )
{
  QMenu *popup = createStandardContextMenu();
  connect( popup, SIGNAL( triggered ( QAction* ) ),
           this, SLOT( menuActivated( QAction* ) ) );

  QList<QAction *> lstAction = popup->actions ();

  if ( !lstAction.isEmpty() ) {
    enum { UndoAct = 0, RedoAct, CutAct, CopyAct, PasteAct, ClearAct, SelectAllAct, NCountActs };
    if ( isReadOnly() )
      lstAction[0]->setIcon( SmallIconSet("editcopy") );
    else {
      lstAction[UndoAct]->setIcon( SmallIconSet("undo") );
      lstAction[RedoAct]->setIcon( SmallIconSet("redo") );
      lstAction[CutAct]->setIcon( SmallIconSet("editcut") );
      lstAction[CopyAct]->setIcon( SmallIconSet("editcopy") );
      lstAction[PasteAct]->setIcon( SmallIconSet("editpaste") );
      lstAction[ClearAct]->setIcon( SmallIconSet("editclear") );
    }
  }

  if( !isReadOnly() ) 
  {
  popup->addSeparator();
  d->spellCheckAction = popup->addAction( SmallIconSet( "spellcheck" ), i18n( "Check Spelling..." ) );

  if ( document()->isEmpty() )
      d->spellCheckAction->setEnabled( false );

  d->autoSpellCheckAction = popup->addAction( i18n( "Auto Spell Check" ) );
  d->autoSpellCheckAction->setCheckable( true );
  d->autoSpellCheckAction->setChecked( d->checkSpellingEnabled );
  popup->addSeparator();

  d->allowTab = popup->addAction( i18n("Allow Tabulations") );
  d->allowTab->setCheckable( true );
  d->allowTab->setChecked( !tabChangesFocus() );
  }
  popup->exec( event->globalPos() );

  delete popup;
}

void KTextEdit::wheelEvent( QWheelEvent *event )
{
  if ( KGlobalSettings::wheelMouseZooms() )
    QTextEdit::wheelEvent( event );
  else // thanks, we don't want to zoom, so skip QTextEdit's impl.
    QAbstractScrollArea::wheelEvent( event );
}

void KTextEdit::setCheckSpellingEnabled( bool check )
{
  if ( check == d->checkSpellingEnabled )
    return;

  // From the above statment we know know that if we're turning checking
  // on that we need to create a new highlighter and if we're turning it
  // off we should remove the old one.

  d->checkSpellingEnabled = check;
#if 0	
    if ( check )
    {
        if ( hasFocus() )
            d->highlighter = new KDictSpellingHighlighter( this );
    }
    else
    {
        delete d->highlighter;
        d->highlighter = 0;
    }
#endif
}

void KTextEdit::focusInEvent( QFocusEvent *event )
{
#if 0
    if ( d->checkSpellingEnabled && !isReadOnly() && !d->highlighter )
        d->highlighter = new KDictSpellingHighlighter( this );
#endif

  QTextEdit::focusInEvent( event );
}

bool KTextEdit::checkSpellingEnabled() const
{
  return d->checkSpellingEnabled;
}

void KTextEdit::setReadOnly( bool readOnly )
{
#if 0
  if ( !readOnly && hasFocus() && d->checkSpellingEnabled && !d->highlighter )
    d->highlighter = new KDictSpellingHighlighter( this );
#endif

  if ( readOnly == isReadOnly() )
    return;

  if ( readOnly ) {
#if 0
    delete d->highlighter;
    d->highlighter = 0;
#endif

    d->customPalette = testAttribute( Qt::WA_SetPalette );
    QPalette p = palette();
    QColor color = p.color( QPalette::Disabled, QPalette::Background );
    p.setColor( QPalette::Base, color );
    p.setColor( QPalette::Background, color );
    setPalette( p );
  } else {
    if ( d->customPalette && testAttribute( Qt::WA_SetPalette ) ) {
        QPalette p = palette();
        QColor color = p.color( QPalette::Normal, QPalette::Base );
        p.setColor( QPalette::Base, color );
        p.setColor( QPalette::Background, color );
        setPalette( p );
    } else
        setPalette( QPalette() );
  }

  QTextEdit::setReadOnly( readOnly );
}

void KTextEdit::checkSpelling()
{
#if 0
  delete d->spell;
  d->spell = new KSpell( this, i18n( "Spell Checking" ),
                         this, SLOT( slotSpellCheckReady( KSpell *) ), 0, true, true);

  connect( d->spell, SIGNAL( death() ),
           this, SLOT( spellCheckerFinished() ) );

  connect( d->spell, SIGNAL( misspelling( const QString &, const QStringList &, unsigned int ) ),
           this, SLOT( spellCheckerMisspelling( const QString &, const QStringList &, unsigned int ) ) );

  connect( d->spell, SIGNAL( corrected( const QString &, const QString &, unsigned int ) ),
           this, SLOT( spellCheckerCorrected( const QString &, const QString &, unsigned int ) ) );
#endif
}

void KTextEdit::highlightWord( unsigned int length, unsigned int pos )
{
/* TODO
  unsigned int l = 0;
  unsigned int cnt = 0;
  posToRowCol( pos, l, cnt );
  setSelection( l, cnt, l, cnt + length );
*/
}

#include "ktextedit.moc"

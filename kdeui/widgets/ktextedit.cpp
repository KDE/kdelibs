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
#include <dialog.h>
#include "backgroundchecker.h"
#include <kaction.h>
#include <kcursor.h>
#include <kglobalsettings.h>
#include <kstandardaction.h>
#include <kstandardshortcut.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdialog.h>
#include <QMenu>
#include <kmessagebox.h>

class KTextEdit::Private
{
  public:
    Private( KTextEdit *_parent )
      : parent( _parent ),
        customPalette( false ),
        checkSpellingEnabled( false ),
        highlighter( 0 )
    {
    }

    ~Private()
    {
      delete highlighter;
    }

    void slotSpellCheckDone( const QString &s );

    void spellCheckerMisspelling( const QString &text, int pos );
    void spellCheckerCorrected( const QString &, int,const QString &);
    void spellCheckerAutoCorrect(const QString&,const QString&);
    void spellCheckerCanceled();

    void spellCheckerFinished();
    void toggleAutoSpellCheck();
    void slotAllowTab();
    void menuActivated( QAction* action );

    KTextEdit *parent;
    QAction *autoSpellCheckAction;
    QAction *allowTab;
    QAction *spellCheckAction;
    bool customPalette : 1;

    bool checkSpellingEnabled : 1;
    QString originalBuffer;
    QString spellChechingConfigFileName;
    QString spellCheckingLanguage;
    Sonnet::Highlighter *highlighter;
};

void KTextEdit::Private::spellCheckerCanceled()
{
    parent->selectAll();
    parent->setPlainText(originalBuffer);
    spellCheckerFinished();
}

void KTextEdit::Private::spellCheckerAutoCorrect(const QString&,const QString&)
{
    //TODO
}

void KTextEdit::Private::slotSpellCheckDone( const QString &s )
{
//Necessary ?
//    if ( s != text() ) // TODO: toPlainText()?? (MiB)
//        setText( s ); // setPlainText() ?! we'd loose rich text info
}

void KTextEdit::Private::spellCheckerMisspelling( const QString &text, int pos )
{
    //kDebug()<<"TextEdit::Private::spellCheckerMisspelling :"<<text<<" pos :"<<pos;
    parent->highlightWord( text.length(), pos );
}

void KTextEdit::Private::spellCheckerCorrected( const QString& oldWord, int pos,const QString& newWord)
{
  //kDebug()<<" oldWord :"<<oldWord<<" newWord :"<<newWord<<" pos : "<<pos;
  if (oldWord != newWord ) {
    QTextCursor cursor(parent->document());
    cursor.setPosition(pos);
    cursor.setPosition(pos+oldWord.length(),QTextCursor::KeepAnchor);
    cursor.insertText(newWord);
  }
}

void KTextEdit::Private::spellCheckerFinished()
{
   QTextCursor cursor(parent->document());
   cursor.clearSelection();
   parent->setTextCursor(cursor);
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

void KTextEdit::setSpellCheckingConfigFileName(const QString &_fileName)
{
    d->spellChechingConfigFileName = _fileName;
}

void KTextEdit::setSpellCheckingLanguage(const QString &_language)
{
    d->spellCheckingLanguage = _language;
} 

void KTextEdit::keyPressEvent( QKeyEvent *event )
{
  int key = event->key() | event->modifiers();

  if ( KStandardShortcut::copy().contains( key ) ) {
    copy();
    event->accept();
    return;
  } else if ( KStandardShortcut::paste().contains( key ) ) {
    paste();
    event->accept();
    return;
  } else if ( KStandardShortcut::cut().contains( key ) ) {
    cut();
    event->accept();
    return;
  } else if ( KStandardShortcut::undo().contains( key ) ) {
    document()->undo();
    event->accept();
    return;
  } else if ( KStandardShortcut::redo().contains( key ) ) {
    document()->redo();
    event->accept();
    return;
  } else if ( KStandardShortcut::deleteWordBack().contains( key ) ) {
    deleteWordBack();
    event->accept();
    return;
  } else if ( KStandardShortcut::deleteWordForward().contains( key ) ) {
    deleteWordForward();
    event->accept();
    return;
  } else if ( KStandardShortcut::backwardWord().contains( key ) ) {
    QTextCursor cursor = textCursor();
    cursor.movePosition( QTextCursor::PreviousWord );
    setTextCursor( cursor );
    event->accept();
    return;
  } else if ( KStandardShortcut::forwardWord().contains( key ) ) {
    QTextCursor cursor = textCursor();
    cursor.movePosition( QTextCursor::NextWord );
    setTextCursor( cursor );
    event->accept();
    return;
  } else if ( KStandardShortcut::next().contains( key ) ) {
    QTextCursor cursor = textCursor();
    int targetY = verticalScrollBar()->value() + viewport()->height();
    bool moved = false;
    do {
      moved = cursor.movePosition( QTextCursor::Down );
      setTextCursor( cursor );
    } while ( moved && verticalScrollBar()->value() < targetY );
    event->accept();
    return;
  } else if ( KStandardShortcut::prior().contains( key ) ) {
    QTextCursor cursor = textCursor();
    int targetY = verticalScrollBar()->value() - viewport()->height();
    bool moved = false;
    do {
      moved = cursor.movePosition( QTextCursor::Up );
      setTextCursor( cursor );
    } while ( moved && verticalScrollBar()->value() > targetY );
    event->accept();
    return;
  } else if ( KStandardShortcut::begin().contains( key ) ) {
    QTextCursor cursor = textCursor();
    cursor.movePosition( QTextCursor::Start );
    setTextCursor( cursor );
    event->accept();
    return;
  } else if ( KStandardShortcut::end().contains( key ) ) {
    QTextCursor cursor = textCursor();
    cursor.movePosition( QTextCursor::End );
    setTextCursor( cursor );
    event->accept();
    return;
  } else if ( KStandardShortcut::beginningOfLine().contains( key ) ) {
    QTextCursor cursor = textCursor();
    cursor.movePosition( QTextCursor::StartOfLine );
    setTextCursor( cursor );
    event->accept();
    return;
  } else if ( KStandardShortcut::endOfLine().contains( key ) ) {
    QTextCursor cursor = textCursor();
    cursor.movePosition( QTextCursor::EndOfLine );
    setTextCursor( cursor );
    event->accept();
    return;
  } else if ( KStandardShortcut::pasteSelection().contains( key ) ) {
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

  if( !isReadOnly() )
  {
      QList<QAction *> actionList = popup->actions();
      enum { UndoAct, RedoAct, CutAct, CopyAct, PasteAct, ClearAct, SelectAllAct, NCountActs };
      QAction *separatorAction = 0L;
      int idx = actionList.indexOf( actionList[SelectAllAct] ) + 1;
      if ( idx < actionList.count() )
          separatorAction = actionList.at( idx );
      if ( separatorAction )
      {
          KAction *clearAllAction = KStandardAction::clear( this, SLOT( clear() ), this) ;
          if ( toPlainText().isEmpty() )
              clearAllAction->setEnabled( false );
          popup->insertAction( separatorAction, clearAllAction );
      }
  }

  KIconTheme::assignIconsToContextMenu( isReadOnly() ? KIconTheme::ReadOnlyText
                                                     : KIconTheme::TextEditor,
                                        popup->actions() );

  if( !isReadOnly() )
  {
      popup->addSeparator();
      if(!acceptRichText())
      {
        d->spellCheckAction = popup->addAction( KIcon( "tools-check-spelling" ), i18n( "Check Spelling..." ) );

        if ( document()->isEmpty() )
           d->spellCheckAction->setEnabled( false );

        d->autoSpellCheckAction = popup->addAction( i18n( "Auto Spell Check" ) );
        d->autoSpellCheckAction->setCheckable( true );
        d->autoSpellCheckAction->setChecked( d->checkSpellingEnabled );
        popup->addSeparator();
      }
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

void KTextEdit::createHighlighter()
{
    setHighlighter(new Sonnet::Highlighter(this, d->spellChechingConfigFileName));
}

Sonnet::Highlighter* KTextEdit::highlighter() const
{
    return d->highlighter;
}

void KTextEdit::setHighlighter(Sonnet::Highlighter *_highLighter)
{
    delete d->highlighter;
    d->highlighter = _highLighter;
}

void KTextEdit::setCheckSpellingEnabled( bool check )
{
  emit checkSpellingChanged( check );
  if ( check == d->checkSpellingEnabled || acceptRichText() )
    return;

  // From the above statment we know know that if we're turning checking
  // on that we need to create a new highlighter and if we're turning it
  // off we should remove the old one.

  d->checkSpellingEnabled = check;
    if ( check )
    {
        if ( hasFocus() ) {
            createHighlighter();
        }
    }
    else
    {
        delete d->highlighter;
        d->highlighter = 0;
    }
}

void KTextEdit::focusInEvent( QFocusEvent *event )
{
    if ( d->checkSpellingEnabled && !isReadOnly() && !d->highlighter && !acceptRichText() )
        createHighlighter();

  QTextEdit::focusInEvent( event );
}

bool KTextEdit::checkSpellingEnabled() const
{
  return d->checkSpellingEnabled;
}

void KTextEdit::setReadOnly( bool readOnly )
{
  if ( !readOnly && hasFocus() && d->checkSpellingEnabled && !d->highlighter  && !acceptRichText())
    createHighlighter();

  if ( readOnly == isReadOnly() )
    return;

  if ( readOnly ) {
    delete d->highlighter;
    d->highlighter = 0;

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
  if(document()->isEmpty())
  {
      KMessageBox::information(this, i18n("Nothing to spell check."));
      return;
  }
  Sonnet::BackgroundChecker *backgroundSpellCheck = new Sonnet::BackgroundChecker(this);
  if(!d->spellCheckingLanguage.isEmpty())
     backgroundSpellCheck->changeLanguage(d->spellCheckingLanguage);
  Sonnet::Dialog *spellDialog = new Sonnet::Dialog(
      backgroundSpellCheck, 0);
  connect(spellDialog, SIGNAL(replace( const QString&, int,const QString&)),
          this, SLOT(spellCheckerCorrected( const QString&, int,const QString&)));
  connect(spellDialog, SIGNAL(misspelling( const QString&, int)),
          this, SLOT(spellCheckerMisspelling(const QString &,int)));
  connect(spellDialog, SIGNAL(autoCorrect(const QString&, const QString&)),
          this, SLOT(spellCheckerAutoCorrect(const QString&, const QString&)));
  connect(spellDialog, SIGNAL(done(const QString&)),
          this, SLOT(spellCheckerFinished()));
  connect(spellDialog, SIGNAL(cancel()),
          this, SLOT(spellCheckerCanceled()));
  connect(spellDialog, SIGNAL(stop()),
          this, SLOT(spellCheckerFinished()));
  connect(spellDialog, SIGNAL(spellCheckStatus(const QString &)),this,SIGNAL(spellCheckStatus(const QString &)));
  d->originalBuffer = toPlainText();
  spellDialog->setBuffer(toPlainText());
  spellDialog->show();
}

void KTextEdit::highlightWord( int length, int pos )
{
  QTextCursor cursor(document());
  cursor.setPosition(pos);
  cursor.setPosition(pos+length,QTextCursor::KeepAnchor);
  setTextCursor (cursor);
}

#include "ktextedit.moc"

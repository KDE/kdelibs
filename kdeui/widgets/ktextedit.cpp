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
#include <kdebug.h>

#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QMenu>
#include <QScrollBar>
#include <QTextCursor>

#include <configdialog.h>
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
#include <kreplacedialog.h>
#include <kfinddialog.h>
#include <kfind.h>
#include <kreplace.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <kwindowsystem.h>

class KTextEdit::Private
{
  public:
    Private( KTextEdit *_parent )
      : parent( _parent ),
        customPalette( false ),
        checkSpellingEnabled( false ),
        findReplaceEnabled(true),
        highlighter( 0 ), findDlg(0),find(0),repDlg(0),replace(0), findIndex(0), repIndex(0)
    {
    }

    ~Private()
    {
      delete highlighter;
      delete findDlg;
      delete find;
      delete replace;
      delete repDlg;
    }

    /**
     * Checks whether we should/should not consume a key used as a shortcut.
     * This makes it possible to handle shortcuts in the focused widget before any
     * window-global QAction is triggered.
     */
    bool overrideShortcut(const QKeyEvent* e);
    /**
     * Actually handle a shortcut event.
     */
    bool handleShortcut(const QKeyEvent* e);

    void spellCheckerMisspelling( const QString &text, int pos );
    void spellCheckerCorrected( const QString &, int,const QString &);
    void spellCheckerAutoCorrect(const QString&,const QString&);
    void spellCheckerCanceled();
    void spellCheckerFinished();
    void toggleAutoSpellCheck();

    void slotFindHighlight(const QString& text, int matchingIndex, int matchingLength);
    void slotReplaceText(const QString &text, int replacementIndex, int /*replacedLength*/, int matchedLength);

    /**
     * Similar to QTextEdit::clear(), only that it is possible to undo this
     * action.
     */
    void undoableClear();

    void slotAllowTab();
    void menuActivated( QAction* action );

    void init();

    KTextEdit *parent;
    KTextEditSpellInterface *spellInterface;
    QAction *autoSpellCheckAction;
    QAction *allowTab;
    QAction *spellCheckAction;
    bool customPalette : 1;

    bool checkSpellingEnabled : 1;
    bool findReplaceEnabled: 1;
    QString originalBuffer;
    QString originalHtml;
    QString spellChechingConfigFileName;
    QString spellCheckingLanguage;
    Sonnet::Highlighter *highlighter;
    KFindDialog *findDlg;
    KFind *find;
    KReplaceDialog *repDlg;
    KReplace *replace;
    int findIndex, repIndex;
};

void KTextEdit::Private::spellCheckerCanceled()
{
    parent->selectAll();
    if(parent->acceptRichText ())
      parent->setHtml(originalHtml);
    else
      parent->setPlainText(originalBuffer);
    spellCheckerFinished();
}

void KTextEdit::Private::spellCheckerAutoCorrect(const QString&,const QString&)
{
    //TODO
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
   if (parent->highlighter())
       parent->highlighter()->rehighlight();
}

void KTextEdit::Private::toggleAutoSpellCheck()
{
  parent->setCheckSpellingEnabled( !parent->checkSpellingEnabled() );
}

void KTextEdit::Private::undoableClear()
{
    QTextCursor cursor = parent->textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    cursor.endEditBlock();
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


void KTextEdit::Private::slotFindHighlight(const QString& text, int matchingIndex, int matchingLength)
{
    Q_UNUSED(text)
    //kDebug() << "Highlight: [" << text << "] mi:" << matchingIndex << " ml:" << matchingLength;
    QTextCursor tc = parent->textCursor();
    tc.setPosition(matchingIndex);
    tc.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, matchingLength);
    parent->setTextCursor(tc);
    parent->ensureCursorVisible();
}


void KTextEdit::Private::slotReplaceText(const QString &text, int replacementIndex, int /*replacedLength*/, int matchedLength) {
    Q_UNUSED(text)
    //kDebug() << "Replace: [" << text << "] ri:" << replacementIndex << " rl:" << replacedLength << " ml:" << matchedLength;
    QTextCursor tc = parent->textCursor();
    tc.setPosition(replacementIndex);
    tc.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, matchedLength);
    tc.removeSelectedText();
    tc.insertText(repDlg->replacement());
    parent->setTextCursor(tc);
    if (replace->options() & KReplaceDialog::PromptOnReplace) {
        parent->ensureCursorVisible();
    }
}

void KTextEdit::Private::init()
{
    spellInterface = 0;
    KCursor::setAutoHideCursor(parent, true, false);
    parent->connect(parent, SIGNAL(languageChanged(const QString&)),
                    parent, SLOT(setSpellCheckingLanguage(const QString&)));
}

KTextEdit::KTextEdit( const QString& text, QWidget *parent )
  : QTextEdit( text, parent ), d( new Private( this ) )
{
  d->init();
}

KTextEdit::KTextEdit( QWidget *parent )
  : QTextEdit( parent ), d( new Private( this ) )
{
  d->init();
}

KTextEdit::~KTextEdit()
{
  delete d;
}

void KTextEdit::setSpellCheckingConfigFileName(const QString &_fileName)
{
    d->spellChechingConfigFileName = _fileName;
}

const QString& KTextEdit::spellCheckingLanguage() const
{
    return d->spellCheckingLanguage;
}

void KTextEdit::setSpellCheckingLanguage(const QString &_language)
{
    if (highlighter()) {
        highlighter()->setCurrentLanguage(_language);
        highlighter()->rehighlight();
    }

    if (_language != d->spellCheckingLanguage) {
        d->spellCheckingLanguage = _language;
        emit languageChanged(_language);
    }
    else
        d->spellCheckingLanguage = _language;
}

void KTextEdit::showSpellConfigDialog(const QString &configFileName,
                                      const QString &windowIcon)
{
    KConfig config(configFileName);
    Sonnet::ConfigDialog dialog(&config, this);
    if (!d->spellCheckingLanguage.isEmpty())
        dialog.setLanguage(d->spellCheckingLanguage);
    connect(&dialog, SIGNAL(languageChanged(const QString &)),
            this, SLOT(setSpellCheckingLanguage(const QString &)));
    if (!windowIcon.isEmpty())
        dialog.setWindowIcon(KIcon(windowIcon));
    dialog.exec();
}

bool KTextEdit::event(QEvent* ev)
{
    if (ev->type() == QEvent::ShortcutOverride) {
        QKeyEvent *e = static_cast<QKeyEvent *>( ev );
        if (d->overrideShortcut(e)) {
            e->accept();
            return true;
        }
    }
    return QTextEdit::event(ev);
}

bool KTextEdit::Private::handleShortcut(const QKeyEvent* event)
{
  const int key = event->key() | event->modifiers();

  if ( KStandardShortcut::copy().contains( key ) ) {
    parent->copy();
    return true;
  } else if ( KStandardShortcut::paste().contains( key ) ) {
    parent->paste();
    return true;
  } else if ( KStandardShortcut::cut().contains( key ) ) {
    parent->cut();
    return true;
  } else if ( KStandardShortcut::undo().contains( key ) ) {
    parent->document()->undo();
    return true;
  } else if ( KStandardShortcut::redo().contains( key ) ) {
    parent->document()->redo();
    return true;
  } else if ( KStandardShortcut::deleteWordBack().contains( key ) ) {
    parent->deleteWordBack();
    return true;
  } else if ( KStandardShortcut::deleteWordForward().contains( key ) ) {
    parent->deleteWordForward();
    return true;
  } else if ( KStandardShortcut::backwardWord().contains( key ) ) {
    QTextCursor cursor = parent->textCursor();
    cursor.movePosition( QTextCursor::PreviousWord );
    parent->setTextCursor( cursor );
    return true;
  } else if ( KStandardShortcut::forwardWord().contains( key ) ) {
    QTextCursor cursor = parent->textCursor();
    cursor.movePosition( QTextCursor::NextWord );
    parent->setTextCursor( cursor );
    return true;
  } else if ( KStandardShortcut::next().contains( key ) ) {
    QTextCursor cursor = parent->textCursor();
    bool moved = false;
    qreal lastY = parent->cursorRect(cursor).bottom();
    qreal distance = 0;
    do {
        qreal y = parent->cursorRect(cursor).bottom();
        distance += qAbs(y - lastY);
        lastY = y;
        moved = cursor.movePosition(QTextCursor::Down);
    } while (moved && distance < parent->viewport()->height());

    if (moved) {
        cursor.movePosition(QTextCursor::Up);
        parent->verticalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepAdd);
    }
    parent->setTextCursor(cursor);
    return true;
  } else if ( KStandardShortcut::prior().contains( key ) ) {
    QTextCursor cursor = parent->textCursor();
    bool moved = false;
    qreal lastY = parent->cursorRect(cursor).bottom();
    qreal distance = 0;
    do {
        qreal y = parent->cursorRect(cursor).bottom();
        distance += qAbs(y - lastY);
        lastY = y;
        moved = cursor.movePosition(QTextCursor::Up);
    } while (moved && distance < parent->viewport()->height());

    if (moved) {
        cursor.movePosition(QTextCursor::Down);
        parent->verticalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepSub);
    }
    parent->setTextCursor(cursor);
    return true;
  } else if ( KStandardShortcut::begin().contains( key ) ) {
    QTextCursor cursor = parent->textCursor();
    cursor.movePosition( QTextCursor::Start );
    parent->setTextCursor( cursor );
    return true;
  } else if ( KStandardShortcut::end().contains( key ) ) {
    QTextCursor cursor = parent->textCursor();
    cursor.movePosition( QTextCursor::End );
    parent->setTextCursor( cursor );
    return true;
  } else if ( KStandardShortcut::beginningOfLine().contains( key ) ) {
    QTextCursor cursor = parent->textCursor();
    cursor.movePosition( QTextCursor::StartOfLine );
    parent->setTextCursor( cursor );
    return true;
  } else if ( KStandardShortcut::endOfLine().contains( key ) ) {
    QTextCursor cursor = parent->textCursor();
    cursor.movePosition( QTextCursor::EndOfLine );
    parent->setTextCursor( cursor );
    return true;
  } else if (KStandardShortcut::find().contains(key)) {
      parent->slotFind();
      return true;
  } else if (KStandardShortcut::findNext().contains(key)) {
      parent->slotFindNext();
      return true;
  } else if (KStandardShortcut::replace().contains(key)) {
      if( !parent->isReadOnly() )
          parent->slotReplace();
      return true;
  } else if ( KStandardShortcut::pasteSelection().contains( key ) ) {
    QString text = QApplication::clipboard()->text( QClipboard::Selection );
    if ( !text.isEmpty() )
      parent->insertPlainText( text );  // TODO: check if this is html? (MiB)
    return true;
  }
  return false;
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

QMenu *KTextEdit::mousePopupMenu()
{
  QMenu *popup = createStandardContextMenu();
  if (!popup) return 0;
  connect( popup, SIGNAL( triggered ( QAction* ) ),
             this, SLOT( menuActivated( QAction* ) ) );

  const bool emptyDocument = document()->isEmpty();
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
          KAction *clearAllAction = KStandardAction::clear(this, SLOT(undoableClear()), popup);
          if ( emptyDocument )
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
      d->spellCheckAction = popup->addAction( KIcon( "tools-check-spelling" ),
                                              i18n( "Check Spelling..." ) );
      if ( emptyDocument )
        d->spellCheckAction->setEnabled( false );
      d->autoSpellCheckAction = popup->addAction( i18n( "Auto Spell Check" ) );
      d->autoSpellCheckAction->setCheckable( true );
      d->autoSpellCheckAction->setChecked( checkSpellingEnabled() );
      popup->addSeparator();
      d->allowTab = popup->addAction( i18n("Allow Tabulations") );
      d->allowTab->setCheckable( true );
      d->allowTab->setChecked( !tabChangesFocus() );

      if (d->findReplaceEnabled)
      {
          KAction *findAction = KStandardAction::find(this, SLOT(slotFind()), popup);
          KAction *findNextAction = KStandardAction::findNext(this, SLOT(slotFindNext()), popup);
          KAction *replaceAction = KStandardAction::replace(this, SLOT(slotReplace()), popup);
          if (emptyDocument)
          {
              findAction->setEnabled(false);
              findNextAction->setEnabled(false );
              replaceAction->setEnabled(false);
          }
	  else
	      findNextAction->setEnabled(d->find != 0 );
          popup->addSeparator();
          popup->addAction(findAction);
          popup->addAction(findNextAction);
          popup->addAction(replaceAction);
      }
  }
  return popup;
}

void KTextEdit::contextMenuEvent(QContextMenuEvent *event)
{
    // Obtain the cursor at the mouse position and the current cursor
    QTextCursor cursorAtMouse = cursorForPosition(event->pos());
    const int mousePos = cursorAtMouse.position();
    QTextCursor cursor = textCursor();

    // Check if the user clicked a selected word
    const bool selectedWordClicked = cursor.hasSelection() &&
                               mousePos >= cursor.selectionStart() &&
                               mousePos <= cursor.selectionEnd();

    // Get the word under the (mouse-)cursor and see if it is misspelled.
    // Don't include apostrophes at the start/end of the word in the selection.
    QTextCursor wordSelectCursor(cursorAtMouse);
    wordSelectCursor.clearSelection();
    wordSelectCursor.select(QTextCursor::WordUnderCursor);
    QString selectedWord = wordSelectCursor.selectedText();

    // Clear the selection again, we re-select it below (without the apostrophes).
    wordSelectCursor.setPosition(wordSelectCursor.position()-selectedWord.size());
    if (selectedWord.startsWith('\'') || selectedWord.startsWith('\"')) {
        selectedWord = selectedWord.right(selectedWord.size() - 1);
        wordSelectCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor);
    }
    if (selectedWord.endsWith('\'') || selectedWord.endsWith('\"'))
        selectedWord.chop(1);

    wordSelectCursor.movePosition(QTextCursor::NextCharacter,
                                  QTextCursor::KeepAnchor, selectedWord.size());

    const bool wordIsMisspelled = checkSpellingEnabled() &&
                            !selectedWord.isEmpty() &&
                            highlighter() &&
                            highlighter()->isWordMisspelled(selectedWord);

    // If the user clicked a selected word, do nothing.
    // If the user clicked somewhere else, move the cursor there.
    // If the user clicked on a misspelled word, select that word.
    // Same behavior as in OpenOffice Writer.
    bool inQuote = false;
    if (d->spellInterface &&
        !d->spellInterface->shouldBlockBeSpellChecked(cursorAtMouse.block().text()))
        inQuote = true;
    if (!selectedWordClicked) {
        if (wordIsMisspelled && !inQuote)
            setTextCursor(wordSelectCursor);
        else
            setTextCursor(cursorAtMouse);
        cursor = textCursor();
    }

    // Use standard context menu for already selected words, correctly spelled
    // words and words inside quotes.
    if (!wordIsMisspelled || selectedWordClicked || inQuote) {
        QMenu *popup = mousePopupMenu();
        if ( popup ) {
            popup->exec( event->globalPos() );
            delete popup;
        }
    }
    else {
        KMenu suggestions;
        KMenu menu;

        //Add the suggestions to the popup menu
        QStringList reps = highlighter()->suggestionsForWord(selectedWord);
        if (reps.count() > 0) {
            for (QStringList::const_iterator it = reps.constBegin(); it != reps.constEnd(); ++it) {
                suggestions.addAction(*it);
            }

        }
        suggestions.setTitle(i18n("Suggestions"));

        QAction *ignoreAction = menu.addAction(i18n("Ignore"));
        QAction *addToDictAction = menu.addAction(i18n("Add to Dictionary"));
        QAction *suggestionsAction = menu.addMenu(&suggestions);
        if (reps.count() == 0)
            suggestionsAction->setEnabled(false);

        //Execute the popup inline
        const QAction *selectedAction = menu.exec(event->globalPos());

        if (selectedAction) {
            Q_ASSERT(cursor.selectedText() == selectedWord);

            if (selectedAction == ignoreAction) {
                highlighter()->ignoreWord(selectedWord);
                highlighter()->rehighlight();
            }
            else if (selectedAction == addToDictAction) {
                highlighter()->addWordToDictionary(selectedWord);
                highlighter()->rehighlight();
            }

            // Other actions can only be one of the suggested words
            else {
                const QString replacement = selectedAction->text();
                Q_ASSERT(reps.contains(replacement));
                cursor.insertText(replacement);
                setTextCursor(cursor);
            }
        }
    }
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

void KTextEdit::setCheckSpellingEnabled(bool check)
{
    if (d->spellInterface)
        d->spellInterface->setSpellCheckingEnabled(check);
    else
        setCheckSpellingEnabledInternal(check);
}

void KTextEdit::setCheckSpellingEnabledInternal( bool check )
{
  emit checkSpellingChanged( check );
  if ( check == d->checkSpellingEnabled )
    return;

  // From the above statment we know know that if we're turning checking
  // on that we need to create a new highlighter and if we're turning it
  // off we should remove the old one.

  d->checkSpellingEnabled = check;
    if ( check )
    {
        if ( hasFocus() ) {
            createHighlighter();
            if (!spellCheckingLanguage().isEmpty())
                setSpellCheckingLanguage(spellCheckingLanguage());
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
  if ( d->checkSpellingEnabled && !isReadOnly() && !d->highlighter )
    createHighlighter();

  QTextEdit::focusInEvent( event );
}

bool KTextEdit::checkSpellingEnabled() const
{
    if (d->spellInterface)
      return d->spellInterface->isSpellCheckingEnabled();
    else
      return checkSpellingEnabledInternal();
}

bool KTextEdit::checkSpellingEnabledInternal() const
{
  return d->checkSpellingEnabled;
}

void KTextEdit::setReadOnly( bool readOnly )
{
  if ( !readOnly && hasFocus() && d->checkSpellingEnabled && !d->highlighter )
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
  connect(spellDialog, SIGNAL(spellCheckStatus(const QString &)),
          this,SIGNAL(spellCheckStatus(const QString &)));
  connect(spellDialog, SIGNAL(languageChanged(const QString &)),
          this, SIGNAL(languageChanged(const QString &)));
  d->originalBuffer = toPlainText();
  if(acceptRichText ())
    d->originalHtml = toHtml();
  spellDialog->setBuffer(d->originalBuffer);
  spellDialog->show();
}

void KTextEdit::highlightWord( int length, int pos )
{
  QTextCursor cursor(document());
  cursor.setPosition(pos);
  cursor.setPosition(pos+length,QTextCursor::KeepAnchor);
  setTextCursor (cursor);
  ensureCursorVisible();
}

void KTextEdit::replace()
{
     if( document()->isEmpty() )  // saves having to track the text changes
        return;

    if ( d->repDlg ) {
      KWindowSystem::activateWindow( d->repDlg->winId() );
    } else {
      d->repDlg = new KReplaceDialog(this, 0,
                                    QStringList(), QStringList(), false);
      connect( d->repDlg, SIGNAL(okClicked()), this, SLOT(slotDoReplace()) );
    }
    d->repDlg->show();
}

void KTextEdit::slotDoReplace()
{
    if (!d->repDlg) {
        // Should really assert()
        return;
    }

    if(d->repDlg->pattern().isEmpty()) {
        d->replace->disconnect(this);
        d->replace->deleteLater(); // we are in a slot connected to m_replace, don't delete it right away
        d->replace = 0;
        ensureCursorVisible();
        return;
    }

    delete d->replace;
    d->replace = new KReplace(d->repDlg->pattern(), d->repDlg->replacement(), d->repDlg->options(), this);
    d->repIndex = 0;
    if (d->replace->options() & KFind::FromCursor || d->replace->options() & KFind::FindBackwards) {
        d->repIndex = textCursor().anchor();
    }

    // Connect highlight signal to code which handles highlighting
    // of found text.
    connect(d->replace, SIGNAL(highlight(const QString &, int, int)),
            this, SLOT(slotFindHighlight(const QString &, int, int)));
    connect(d->replace, SIGNAL(findNext()), this, SLOT(slotReplaceNext()));
    connect(d->replace, SIGNAL(replace(const QString &, int, int, int)),
            this, SLOT(slotReplaceText(const QString &, int, int, int)));

    d->repDlg->close();
    slotReplaceNext();
}


void KTextEdit::slotReplaceNext()
{
    if (!d->replace)
        return;

    if (!(d->replace->options() & KReplaceDialog::PromptOnReplace))
        viewport()->setUpdatesEnabled(false);

    KFind::Result res = KFind::NoMatch;

    if (d->replace->needData())
        d->replace->setData(toPlainText(), d->repIndex);
    res = d->replace->replace();
    if (!(d->replace->options() & KReplaceDialog::PromptOnReplace)) {
        viewport()->setUpdatesEnabled(true);
        viewport()->update();
    }

    if (res == KFind::NoMatch) {
        d->replace->displayFinalDialog();
        d->replace->disconnect(this);
        d->replace->deleteLater(); // we are in a slot connected to m_replace, don't delete it right away
        d->replace = 0;
        ensureCursorVisible();
        //or           if ( m_replace->shouldRestart() ) { reinit (w/o FromCursor) and call slotReplaceNext(); }
    } else {
        //m_replace->closeReplaceNextDialog();
    }
}


void KTextEdit::slotDoFind()
{
    if (!d->findDlg) {
        // Should really assert()
        return;
    }
    if( d->findDlg->pattern().isEmpty())
    {
        d->find->disconnect(this);
        d->find->deleteLater(); // we are in a slot connected to m_find, don't delete right away
        d->find = 0;
        return;
    }
    delete d->find;
    d->find = new KFind(d->findDlg->pattern(), d->findDlg->options(), this);
    d->findIndex = 0;
    if (d->find->options() & KFind::FromCursor || d->find->options() & KFind::FindBackwards) {
        d->findIndex = textCursor().anchor();
    }

    // Connect highlight signal to code which handles highlighting
    // of found text.
    connect(d->find, SIGNAL(highlight(const QString &, int, int)),
            this, SLOT(slotFindHighlight(const QString &, int, int)));
    connect(d->find, SIGNAL(findNext()), this, SLOT(slotFindNext()));

    d->findDlg->close();
    d->find->closeFindNextDialog();
    slotFindNext();
}


void KTextEdit::slotFindNext()
{
    if (!d->find)
        return;
    if(document()->isEmpty())
    {
        d->find->disconnect(this);
        d->find->deleteLater(); // we are in a slot connected to m_find, don't delete right away
        d->find = 0;
        return;
    }

    KFind::Result res = KFind::NoMatch;
    if (d->find->needData())
        d->find->setData(toPlainText(), d->findIndex);
    res = d->find->find();

    if (res == KFind::NoMatch) {
        d->find->displayFinalDialog();
        d->find->disconnect(this);
        d->find->deleteLater(); // we are in a slot connected to m_find, don't delete right away
        d->find = 0;
        //or           if ( m_find->shouldRestart() ) { reinit (w/o FromCursor) and call slotFindNext(); }
    } else {
        //m_find->closeFindNextDialog();
    }
}


void KTextEdit::slotFind()
{
    if( document()->isEmpty() )  // saves having to track the text changes
        return;

    if ( d->findDlg ) {
      KWindowSystem::activateWindow( d->findDlg->winId() );
    } else {
      d->findDlg = new KFindDialog(this);
      connect( d->findDlg, SIGNAL(okClicked()), this, SLOT(slotDoFind()) );
    }
    d->findDlg->show();
}


void KTextEdit::slotReplace()
{
    if( document()->isEmpty() )  // saves having to track the text changes
        return;

    if ( d->repDlg ) {
      KWindowSystem::activateWindow( d->repDlg->winId() );
    } else {
      d->repDlg = new KReplaceDialog(this, 0,
                                    QStringList(), QStringList(), false);
      connect( d->repDlg, SIGNAL(okClicked()), this, SLOT(slotDoReplace()) );
    }
    d->repDlg->show();
}

void KTextEdit::enableFindReplace( bool enabled )
{
    d->findReplaceEnabled = enabled;
}

void KTextEdit::setSpellInterface(KTextEditSpellInterface *spellInterface)
{
    d->spellInterface = spellInterface;
}

bool KTextEdit::Private::overrideShortcut(const QKeyEvent* event)
{
  const int key = event->key() | event->modifiers();

  if ( KStandardShortcut::copy().contains( key ) ) {
    return true;
  } else if ( KStandardShortcut::paste().contains( key ) ) {
    return true;
  } else if ( KStandardShortcut::cut().contains( key ) ) {
    return true;
  } else if ( KStandardShortcut::undo().contains( key ) ) {
    return true;
  } else if ( KStandardShortcut::redo().contains( key ) ) {
    return true;
  } else if ( KStandardShortcut::deleteWordBack().contains( key ) ) {
    return true;
  } else if ( KStandardShortcut::deleteWordForward().contains( key ) ) {
    return true;
  } else if ( KStandardShortcut::backwardWord().contains( key ) ) {
    return true;
  } else if ( KStandardShortcut::forwardWord().contains( key ) ) {
    return true;
  } else if ( KStandardShortcut::next().contains( key ) ) {
    return true;
  } else if ( KStandardShortcut::prior().contains( key ) ) {
    return true;
  } else if ( KStandardShortcut::begin().contains( key ) ) {
    return true;
  } else if ( KStandardShortcut::end().contains( key ) ) {
    return true;
  } else if ( KStandardShortcut::beginningOfLine().contains( key ) ) {
    return true;
  } else if ( KStandardShortcut::endOfLine().contains( key ) ) {
    return true;
  } else if ( KStandardShortcut::pasteSelection().contains( key ) ) {
    return true;
  } else if (KStandardShortcut::find().contains(key)) {
      return true;
  } else if (KStandardShortcut::findNext().contains(key)) {
      return true;
  } else if (KStandardShortcut::replace().contains(key)) {
      return true;
  } else if (event->matches(QKeySequence::SelectAll)) { // currently missing in QTextEdit
      return true;
  } else if (event->modifiers() == Qt::ControlModifier &&
            (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) &&
              qobject_cast<KDialog*>(parent->window()) ) {
    // ignore Ctrl-Return so that KDialogs can close the dialog
    return true;
  }
  return false;
}

void KTextEdit::keyPressEvent( QKeyEvent *event )
{
    if (d->handleShortcut(event)) {
        event->accept();
    }else if (event->modifiers() == Qt::ControlModifier &&
            (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) &&
              qobject_cast<KDialog*>(window()) ) {
        event->ignore();
    } else {
        QTextEdit::keyPressEvent(event);
    }
}

#include "ktextedit.moc"

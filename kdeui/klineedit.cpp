/* This file is part of the KDE libraries

   Copyright (C) 1997 Sven Radej (sven.radej@iname.com)
   Copyright (c) 1999 Patrick Ward <PAT_WARD@HP-USA-om5.om.hp.com>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>

   Re-designed for KDE 2.x by
   Copyright (c) 2000, 2001 Dawit Alemayehu <adawit@kde.org>
   Copyright (c) 2000, 2001 Carsten Pfeiffer <pfeiffer@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later
   version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qclipboard.h>
#include <qtimer.h>

#include <kconfig.h>
#include <qtooltip.h>
#include <kcursor.h>
#include <klocale.h>
#include <kstdaccel.h>
#include <kpopupmenu.h>
#include <kdebug.h>
#include <kcompletionbox.h>
#include <kurl.h>
#include <kurldrag.h>
#include <kiconloader.h>
#include <kapplication.h>

#include "klineedit.h"
#include "klineedit.moc"


class KLineEdit::KLineEditPrivate
{
public:
    KLineEditPrivate()
    {
        completionBox = 0L;
        handleURLDrops = true;
        grabReturnKeyEvents = false;

        userSelection = true;
        autoSuggest = false;
        disableRestoreSelection = false;
        enableSqueezedText = false;

        if ( !initialized )
        {
            KConfigGroup config( KGlobal::config(), "General" );
            backspacePerformsCompletion = config.readBoolEntry( "Backspace performs completion", false );

            initialized = true;
        }

    }

    ~KLineEditPrivate()
    {
// causes a weird crash in KWord at least, so let Qt delete it for us.
//        delete completionBox;
    }

    static bool initialized;
    static bool backspacePerformsCompletion; // Configuration option

    QColor previousHighlightColor;
    QColor previousHighlightedTextColor;

    bool userSelection: 1;
    bool autoSuggest : 1;
    bool disableRestoreSelection: 1;
    bool handleURLDrops:1;
    bool grabReturnKeyEvents:1;
    bool enableSqueezedText:1;

    int squeezedEnd;
    int squeezedStart;
    BackgroundMode bgMode;
    QString squeezedText;
    KCompletionBox *completionBox;
};

bool KLineEdit::KLineEditPrivate::backspacePerformsCompletion = false;
bool KLineEdit::KLineEditPrivate::initialized = false;


KLineEdit::KLineEdit( const QString &string, QWidget *parent, const char *name )
          :QLineEdit( string, parent, name )
{
    init();
}

KLineEdit::KLineEdit( QWidget *parent, const char *name )
          :QLineEdit( parent, name )
{
    init();
}

KLineEdit::~KLineEdit ()
{
    delete d;
    d = 0;
}

void KLineEdit::init()
{
    d = new KLineEditPrivate;
    possibleTripleClick = false;
    d->bgMode = backgroundMode ();

    // Enable the context menu by default.
    KLineEdit::setContextMenuEnabled( true );
    KCursor::setAutoHideCursor( this, true, true );
    installEventFilter( this );

    KGlobalSettings::Completion mode = completionMode();
    d->autoSuggest = (mode == KGlobalSettings::CompletionMan ||
                      mode == KGlobalSettings::CompletionPopupAuto ||
                      mode == KGlobalSettings::CompletionAuto);
    connect( this, SIGNAL(selectionChanged()), this, SLOT(slotRestoreSelectionColors()));

    QPalette p = palette();
    if ( !d->previousHighlightedTextColor.isValid() )
      d->previousHighlightedTextColor=p.color(QPalette::Normal,QColorGroup::HighlightedText);
    if ( !d->previousHighlightColor.isValid() )
      d->previousHighlightColor=p.color(QPalette::Normal,QColorGroup::Highlight);
}

void KLineEdit::setCompletionMode( KGlobalSettings::Completion mode )
{
    KGlobalSettings::Completion oldMode = completionMode();

    if ( oldMode != mode && (oldMode == KGlobalSettings::CompletionPopup ||
         oldMode == KGlobalSettings::CompletionPopupAuto ) &&
         d->completionBox && d->completionBox->isVisible() )
      d->completionBox->hide();

    // If the widgets echo mode is not Normal, no completion
    // feature will be enabled even if one is requested.
    if ( echoMode() != QLineEdit::Normal )
        mode = KGlobalSettings::CompletionNone; // Override the request.

    if ( kapp && !kapp->authorize("lineedit_text_completion") )
        mode = KGlobalSettings::CompletionNone;

    if ( mode == KGlobalSettings::CompletionPopupAuto ||
         mode == KGlobalSettings::CompletionAuto ||
         mode == KGlobalSettings::CompletionMan )
        d->autoSuggest = true;
    else
        d->autoSuggest = false;

    KCompletionBase::setCompletionMode( mode );
}

void KLineEdit::setCompletedText( const QString& t, bool marked )
{
    if ( !d->autoSuggest )
      return;

    QString txt = text();

    if ( t != txt )
    {
        int start = marked ? txt.length() : t.length();
        validateAndSet( t, cursorPosition(), start, t.length() );
        setUserSelection(false);
    }
    else
      setUserSelection(true);

}

void KLineEdit::setCompletedText( const QString& text )
{
    KGlobalSettings::Completion mode = completionMode();
    bool marked = ( mode == KGlobalSettings::CompletionAuto ||
                    mode == KGlobalSettings::CompletionMan ||
                    mode == KGlobalSettings::CompletionPopup ||
                    mode == KGlobalSettings::CompletionPopupAuto );
    setCompletedText( text, marked );
}

void KLineEdit::rotateText( KCompletionBase::KeyBindingType type )
{
    KCompletion* comp = compObj();
    if ( comp &&
       (type == KCompletionBase::PrevCompletionMatch ||
        type == KCompletionBase::NextCompletionMatch ) )
    {
       QString input;

       if (type == KCompletionBase::PrevCompletionMatch)
          comp->previousMatch();
       else
          comp->nextMatch();

       // Skip rotation if previous/next match is null or the same text
       if ( input.isNull() || input == displayText() )
            return;
       setCompletedText( input, hasSelectedText() );
    }
}

void KLineEdit::makeCompletion( const QString& text )
{
    KCompletion *comp = compObj();
    KGlobalSettings::Completion mode = completionMode();

    if ( !comp || mode == KGlobalSettings::CompletionNone )
        return;  // No completion object...

    QString match = comp->makeCompletion( text );

    if ( mode == KGlobalSettings::CompletionPopup ||
         mode == KGlobalSettings::CompletionPopupAuto )
    {
        if ( match.isNull() )
        {
            if ( d->completionBox )
            {
                d->completionBox->hide();
                d->completionBox->clear();
            }
        }
        else
            setCompletedItems( comp->allMatches() );
    }
    else // Auto,  ShortAuto (Man) and Shell
    {
        // all other completion modes
        // If no match or the same match, simply return without completing.
        if ( match.isNull() || match == text )
            return;

        if ( mode != KGlobalSettings::CompletionShell )
            setUserSelection(false);

        if ( d->autoSuggest )
            setCompletedText( match );
    }
}

void KLineEdit::setReadOnly(bool readOnly)
{
    // Do not do anything if nothing changed...
    if (readOnly == isReadOnly ())
      return;

    QLineEdit::setReadOnly (readOnly);

    if (readOnly)
    {
        d->bgMode = backgroundMode ();
        setBackgroundMode (Qt::PaletteBackground);
        if (d->enableSqueezedText && d->squeezedText.isEmpty())
        {
            d->squeezedText = text();
            setSqueezedText();
        }
    }
    else
    {
        if (!d->squeezedText.isEmpty())
        {
           setText(d->squeezedText);
           d->squeezedText = QString::null;
        }
        setBackgroundMode (d->bgMode);
    }
}

void KLineEdit::setSqueezedText( const QString &text)
{
    setEnableSqueezedText(true);
    setText(text);
}

void KLineEdit::setEnableSqueezedText( bool enable )
{
    d->enableSqueezedText = enable;
}

bool KLineEdit::isSqueezedTextEnabled() const
{
    return d->enableSqueezedText;
}

void KLineEdit::setText( const QString& text )
{
    if( d->enableSqueezedText && isReadOnly() )
    {
        d->squeezedText = text;
        setSqueezedText();
        return;
    }

    QLineEdit::setText( text );
}

void KLineEdit::setSqueezedText()
{
    d->squeezedStart = 0;
    d->squeezedEnd = 0;
    QString fullText = d->squeezedText;
    QFontMetrics fm(fontMetrics());
    int labelWidth = size().width() - 2*frameWidth() - 2;
    int textWidth = fm.width(fullText);

    if (textWidth > labelWidth)
    {
          // start with the dots only
          QString squeezedText = "...";
          int squeezedWidth = fm.width(squeezedText);

          // estimate how many letters we can add to the dots on both sides
          int letters = fullText.length() * (labelWidth - squeezedWidth) / textWidth / 2;
          squeezedText = fullText.left(letters) + "..." + fullText.right(letters);
          squeezedWidth = fm.width(squeezedText);

      if (squeezedWidth < labelWidth)
      {
             // we estimated too short
             // add letters while text < label
          do
          {
                letters++;
                squeezedText = fullText.left(letters) + "..." + fullText.right(letters);
                squeezedWidth = fm.width(squeezedText);
             } while (squeezedWidth < labelWidth);
             letters--;
             squeezedText = fullText.left(letters) + "..." + fullText.right(letters);
      }
      else if (squeezedWidth > labelWidth)
      {
             // we estimated too long
             // remove letters while text > label
          do
          {
               letters--;
                squeezedText = fullText.left(letters) + "..." + fullText.right(letters);
                squeezedWidth = fm.width(squeezedText);
             } while (squeezedWidth > labelWidth);
          }

      if (letters < 5)
      {
             // too few letters added -> we give up squeezing
          QLineEdit::setText(fullText);
      }
      else
      {
          QLineEdit::setText(squeezedText);
             d->squeezedStart = letters;
             d->squeezedEnd = fullText.length() - letters;
          }

          QToolTip::remove( this );
          QToolTip::add( this, fullText );

    }
    else
    {
      QLineEdit::setText(fullText);

          QToolTip::remove( this );
          QToolTip::hide();
       }

       setCursorPosition(0);
}

void KLineEdit::copy() const
{
   if (!d->squeezedText.isEmpty() && d->squeezedStart)
   {
      int start, end;
      KLineEdit *that = const_cast<KLineEdit *>(this);
      if (!that->getSelection(&start, &end))
         return;
      if (start >= d->squeezedStart+3)
         start = start - 3 - d->squeezedStart + d->squeezedEnd;
      else if (start > d->squeezedStart)
         start = d->squeezedStart;
      if (end >= d->squeezedStart+3)
         end = end - 3 - d->squeezedStart + d->squeezedEnd;
      else if (end > d->squeezedStart)
         end = d->squeezedEnd;
      if (start == end)
         return;
      QString t = d->squeezedText;
      t = t.mid(start, end - start);
      disconnect( QApplication::clipboard(), SIGNAL(selectionChanged()), this, 0);
      QApplication::clipboard()->setText( t );
      connect( QApplication::clipboard(), SIGNAL(selectionChanged()), this,
               SLOT(clipboardChanged()) );
      return;
   }

   QLineEdit::copy();
}

void KLineEdit::resizeEvent( QResizeEvent * ev )
{
    if (!d->squeezedText.isEmpty())
        setSqueezedText();

    QLineEdit::resizeEvent(ev);
}

void KLineEdit::keyPressEvent( QKeyEvent *e )
{
    KKey key( e );

    if ( KStdAccel::copy().contains( key ) )
    {
        copy();
        return;
    }
    else if ( KStdAccel::paste().contains( key ) )
    {
        paste();
        return;
    }

    // support for pasting Selection with Shift-Ctrl-Insert
    else if ( e->key() == Key_Insert &&
              (e->state() == (ShiftButton | ControlButton)) )
    {
#if QT_VERSION >= 0x030100
        QString text = QApplication::clipboard()->text( QClipboard::Selection);
#else
        QClipboard *clip = QApplication::clipboard();
        bool oldMode = clip->selectionModeEnabled();
        clip->setSelectionMode( true );
        QString text = QApplication::clipboard()->text();
        clip->setSelectionMode( oldMode );
#endif

        insert( text );
        deselect();
        return;
    }

    else if ( KStdAccel::cut().contains( key ) )
    {
        cut();
        return;
    }
    else if ( KStdAccel::undo().contains( key ) )
    {
        undo();
        return;
    }
    else if ( KStdAccel::redo().contains( key ) )
    {
        redo();
        return;
    }
    else if ( KStdAccel::deleteWordBack().contains( key ) )
    {
        cursorWordBackward(true);
        if ( hasSelectedText() )
            del();

        e->accept();
        return;
    }
    else if ( KStdAccel::deleteWordForward().contains( key ) )
    {
        // Workaround for QT bug where
        cursorWordForward(true);
        if ( hasSelectedText() )
            del();

        e->accept();
        return;
    }
    else if ( KStdAccel::backwardWord().contains( key ) )
    {
      cursorWordBackward(false);
      e->accept();
      return;
    }
    else if ( KStdAccel::forwardWord().contains( key ) )
    {
      cursorWordForward(false);
      e->accept();
      return;
    }
    else if ( KStdAccel::beginningOfLine().contains( key ) )
    {
      home(false);
      e->accept();
      return;
    }
    else if ( KStdAccel::endOfLine().contains( key ) )
    {
      end(false);
      e->accept();
      return;
    }


    // Filter key-events if EchoMode is normal and
    // completion mode is not set to CompletionNone
    if ( echoMode() == QLineEdit::Normal &&
         completionMode() != KGlobalSettings::CompletionNone )
    {
        KeyBindingMap keys = getKeyBindings();
        KGlobalSettings::Completion mode = completionMode();
        bool noModifier = (e->state() == NoButton ||
                           e->state() == ShiftButton ||
                           e->state() == Keypad);

        if ( (mode == KGlobalSettings::CompletionAuto ||
              mode == KGlobalSettings::CompletionPopupAuto ||
              mode == KGlobalSettings::CompletionMan) && noModifier )
        {
            if ( !d->userSelection && hasSelectedText() &&
                 ( e->key() == Key_Right || e->key() == Key_Left ) &&
                 e->state()==NoButton )
            {
                QString old_txt = text();
                d->disableRestoreSelection = true;
                int start,end;
                getSelection(&start, &end);

                deselect();
                QLineEdit::keyPressEvent ( e );
                int cPosition=cursorPosition();
                if (e->key() ==Key_Right && cPosition > start )
                    validateAndSet(old_txt, cPosition, cPosition, old_txt.length());
                else
                    validateAndSet(old_txt, cPosition, start, old_txt.length());

                d->disableRestoreSelection = false;
                return;
            }

            if ( e->key() == Key_Escape )
            {
                if (hasSelectedText() && !d->userSelection )
                {
                    del();
                    setUserSelection(true);
                }

                // Don't swallow the Escape press event for the case
                // of dialogs, which have Escape associated to Cancel
                e->ignore();
                return;
            }

        }

        if ( (mode == KGlobalSettings::CompletionAuto ||
              mode == KGlobalSettings::CompletionMan) && noModifier )
        {
            QString keycode = e->text();
            if ( !keycode.isEmpty() && (keycode.unicode()->isPrint() ||
                e->key() == Key_Backspace || e->key() == Key_Delete ) )
            {
                bool hasUserSelection=d->userSelection;
                bool hadSelection=hasSelectedText();

                bool cursorNotAtEnd=false;

                int start,end;
                getSelection(&start, &end);
                int cPos = cursorPosition();

                // When moving the cursor, we want to keep the autocompletion as an
                // autocompletion, so we want to process events at the cursor position
                // as if there was no selection. After processing the key event, we
                // can set the new autocompletion again.
                if ( hadSelection && !hasUserSelection && start>cPos )
                {
                    del();
                    setCursorPosition(cPos);
                    cursorNotAtEnd=true;
                }

                d->disableRestoreSelection = true;
                QLineEdit::keyPressEvent ( e );
                d->disableRestoreSelection = false;

                QString txt = text();
                int len = txt.length();
                if ( !hasSelectedText() && len /*&& cursorPosition() == len */)
                {
                    if ( e->key() == Key_Backspace )
                    {
                        if ( hadSelection && !hasUserSelection && !cursorNotAtEnd )
                        {
                            backspace();
                            txt = text();
                            len = txt.length();
                        }

                        if ( !d->backspacePerformsCompletion || !len )
                            d->autoSuggest = false;
                    }

                    if (e->key() == Key_Delete )
                        d->autoSuggest=false;

                    if ( emitSignals() )
                        emit completion( txt );

                    if ( handleSignals() )
                        makeCompletion( txt );

                    if(  (e->key() == Key_Backspace || e->key() == Key_Delete) )
                        d->autoSuggest=true;

                    e->accept();
                }

                return;
            }

        }

        else if (( mode == KGlobalSettings::CompletionPopup ||
                   mode == KGlobalSettings::CompletionPopupAuto ) &&
                   noModifier && !e->text().isEmpty() )
        {
            QString old_txt = text();
            bool hasUserSelection=d->userSelection;
            bool hadSelection=hasSelectedText();
            bool cursorNotAtEnd=false;

            int start,end;
            getSelection(&start, &end);
            int cPos = cursorPosition();
            QString keycode = e->text();

            // When moving the cursor, we want to keep the autocompletion as an
            // autocompletion, so we want to process events at the cursor position
            // as if there was no selection. After processing the key event, we
            // can set the new autocompletion again.
            if (hadSelection && !hasUserSelection && start>cPos &&
               ( (!keycode.isEmpty() && keycode.unicode()->isPrint()) ||
                 e->key() == Key_Backspace || e->key() == Key_Delete ) )
            {
                del();
                setCursorPosition(cPos);
                cursorNotAtEnd=true;
            }

            uint selectedLength=selectedText().length();

            d->disableRestoreSelection = true;
            QLineEdit::keyPressEvent ( e );
            d->disableRestoreSelection = false;

            if (( selectedLength != selectedText().length() ) && !hasUserSelection )
                slotRestoreSelectionColors(); // and set userSelection to true

            QString txt = text();
            int len = txt.length();

            if ( txt != old_txt && len/* && ( cursorPosition() == len || force )*/ &&
                 ( (!keycode.isEmpty() && keycode.unicode()->isPrint()) ||
                   e->key() == Key_Backspace || e->key() == Key_Delete) )
            {
                if ( e->key() == Key_Backspace )
                {
                    if ( hadSelection && !hasUserSelection && !cursorNotAtEnd )
                    {
                        backspace();
                        txt = text();
                        len = txt.length();
                    }

                    if ( !d->backspacePerformsCompletion )
                        d->autoSuggest = false;
                }

                if (e->key() == Key_Delete )
                    d->autoSuggest=false;

                if ( emitSignals() )
                  emit completion( txt ); // emit when requested...

                if ( handleSignals() )
                  makeCompletion( txt );  // handle when requested...

                if ( (e->key() == Key_Backspace || e->key() == Key_Delete ) &&
                    mode == KGlobalSettings::CompletionPopupAuto )
                  d->autoSuggest=true;

                e->accept();
            }
            else if (!len && d->completionBox && d->completionBox->isVisible())
                d->completionBox->hide();

            return;
        }

        else if ( mode == KGlobalSettings::CompletionShell )
        {
            // Handles completion.
            KShortcut cut;
            if ( keys[TextCompletion].isNull() )
                cut = KStdAccel::shortcut(KStdAccel::TextCompletion);
            else
                cut = keys[TextCompletion];

            if ( cut.contains( key ) )
            {
                // Emit completion if the completion mode is CompletionShell
                // and the cursor is at the end of the string.
                QString txt = text();
                int len = txt.length();
                if ( cursorPosition() == len && len != 0 )
                {
                    if ( emitSignals() )
                        emit completion( txt );
                    if ( handleSignals() )
                        makeCompletion( txt );
                    return;
                }
            }
            else if ( d->completionBox )
                d->completionBox->hide();
        }

        // handle rotation
        if ( mode != KGlobalSettings::CompletionNone )
        {
            // Handles previous match
            KShortcut cut;
            if ( keys[PrevCompletionMatch].isNull() )
                cut = KStdAccel::shortcut(KStdAccel::PrevCompletion);
            else
                cut = keys[PrevCompletionMatch];

            if ( cut.contains( key ) )
            {
                if ( emitSignals() )
                    emit textRotation( KCompletionBase::PrevCompletionMatch );
                if ( handleSignals() )
                    rotateText( KCompletionBase::PrevCompletionMatch );
                return;
            }

            // Handles next match
            if ( keys[NextCompletionMatch].isNull() )
                cut = KStdAccel::shortcut(KStdAccel::NextCompletion);
            else
                cut = keys[NextCompletionMatch];

            if ( cut.contains( key ) )
            {
                if ( emitSignals() )
                    emit textRotation( KCompletionBase::NextCompletionMatch );
                if ( handleSignals() )
                    rotateText( KCompletionBase::NextCompletionMatch );
                return;
            }
        }

        // substring completion
        if ( compObj() )
        {
            KShortcut cut;
            if ( keys[SubstringCompletion].isNull() )
                cut = KStdAccel::shortcut(KStdAccel::SubstringCompletion);
            else
                cut = keys[SubstringCompletion];

            if ( cut.contains( key ) )
            {
                if ( emitSignals() )
                    emit substringCompletion( text() );
                if ( handleSignals() )
                {
                    setCompletedItems( compObj()->substringCompletion(text()));
                    e->accept();
                }
                return;
            }
        }
    }

    uint selectedLength = selectedText().length();

    // Let QLineEdit handle any other keys events.
    QLineEdit::keyPressEvent ( e );

    if ( selectedLength != selectedText().length() )
        slotRestoreSelectionColors(); // and set userSelection to true
}

void KLineEdit::mouseDoubleClickEvent( QMouseEvent* e )
{
    if ( e->button() == Qt::LeftButton  )
    {
        possibleTripleClick=true;
        QTimer::singleShot( QApplication::doubleClickInterval(),this,
                            SLOT(tripleClickTimeout()) );
    }
    QLineEdit::mouseDoubleClickEvent( e );
}

void KLineEdit::mousePressEvent( QMouseEvent* e )
{
    if ( possibleTripleClick && e->button() == Qt::LeftButton )
    {
        selectAll();
        e->accept();
        return;
    }
    QLineEdit::mousePressEvent( e );
}

void KLineEdit::tripleClickTimeout()
{
    possibleTripleClick=false;
}

void KLineEdit::contextMenuEvent( QContextMenuEvent * e )
{
    if ( m_bEnableMenu )
        QLineEdit::contextMenuEvent( e );
}

QPopupMenu *KLineEdit::createPopupMenu()
{
    enum { IdUndo, IdRedo, IdSep1, IdCut, IdCopy, IdPaste, IdClear, IdSep2, IdSelectAll };

    QPopupMenu *popup = QLineEdit::createPopupMenu();

    if ( isReadOnly() )
      popup->changeItem( popup->idAt(0), SmallIconSet("editcopy"), popup->text( popup->idAt(0) ) );
    else {
      int id = popup->idAt(0);
      popup->changeItem( id - IdUndo, SmallIconSet("undo"), popup->text( id - IdUndo) );
      popup->changeItem( id - IdRedo, SmallIconSet("redo"), popup->text( id - IdRedo) );
      popup->changeItem( id - IdCut, SmallIconSet("editcut"), popup->text( id - IdCut) );
      popup->changeItem( id - IdCopy, SmallIconSet("editcopy"), popup->text( id - IdCopy) );
      popup->changeItem( id - IdPaste, SmallIconSet("editpaste"), popup->text( id - IdPaste) );
      popup->changeItem( id - IdClear, SmallIconSet("editclear"), popup->text( id - IdClear) );
    }
      
    // If a completion object is present and the input
    // widget is not read-only, show the Text Completion
    // menu item.
    if ( compObj() && !isReadOnly() && kapp->authorize("lineedit_text_completion") )
    {
        QPopupMenu *subMenu = new QPopupMenu( popup );
        connect( subMenu, SIGNAL( activated( int ) ),
                 this, SLOT( completionMenuActivated( int ) ) );

        popup->insertSeparator();
        popup->insertItem( SmallIconSet("completion"), i18n("Text Completion"),
                           subMenu );

        subMenu->insertItem( i18n("None"), NoCompletion );
        subMenu->insertItem( i18n("Manual"), ShellCompletion );
        subMenu->insertItem( i18n("Automatic"), AutoCompletion );
        subMenu->insertItem( i18n("Dropdown List"), PopupCompletion );
        subMenu->insertItem( i18n("Short Automatic"), ShortAutoCompletion );
        subMenu->insertItem( i18n("Dropdown List && Automatic"), PopupAutoCompletion );

        subMenu->setAccel( KStdAccel::completion(), ShellCompletion );

        KGlobalSettings::Completion mode = completionMode();
        subMenu->setItemChecked( NoCompletion,
                                 mode == KGlobalSettings::CompletionNone );
        subMenu->setItemChecked( ShellCompletion,
                                 mode == KGlobalSettings::CompletionShell );
        subMenu->setItemChecked( PopupCompletion,
                                 mode == KGlobalSettings::CompletionPopup );
        subMenu->setItemChecked( AutoCompletion,
                                 mode == KGlobalSettings::CompletionAuto );
        subMenu->setItemChecked( ShortAutoCompletion,
                                 mode == KGlobalSettings::CompletionMan );
        subMenu->setItemChecked( PopupAutoCompletion,
                                 mode == KGlobalSettings::CompletionPopupAuto );
        if ( mode != KGlobalSettings::completionMode() )
        {
            subMenu->insertSeparator();
            subMenu->insertItem( i18n("Default"), Default );
        }
    }

    // ### do we really need this?  Yes, Please do not remove!  This
    // allows applications to extend the popup menu without having to
    // inherit from this class! (DA)
    emit aboutToShowContextMenu( popup );

    return popup;
}

void KLineEdit::completionMenuActivated( int id )
{
    KGlobalSettings::Completion oldMode = completionMode();

    switch ( id )
    {
        case Default:
           setCompletionMode( KGlobalSettings::completionMode() );
           break;
        case NoCompletion:
           setCompletionMode( KGlobalSettings::CompletionNone );
           break;
        case AutoCompletion:
            setCompletionMode( KGlobalSettings::CompletionAuto );
            break;
        case ShortAutoCompletion:
            setCompletionMode( KGlobalSettings::CompletionMan );
            break;
        case ShellCompletion:
            setCompletionMode( KGlobalSettings::CompletionShell );
            break;
        case PopupCompletion:
            setCompletionMode( KGlobalSettings::CompletionPopup );
            break;
        case PopupAutoCompletion:
            setCompletionMode( KGlobalSettings::CompletionPopupAuto );
            break;
        default:
            return;
    }

    if ( oldMode != completionMode() )
    {
        if ( (oldMode == KGlobalSettings::CompletionPopup ||
              oldMode == KGlobalSettings::CompletionPopupAuto ) &&
              d->completionBox && d->completionBox->isVisible() )
            d->completionBox->hide();
        emit completionModeChanged( completionMode() );
    }
}

void KLineEdit::dropEvent(QDropEvent *e)
{
    KURL::List urlList;
    if( d->handleURLDrops && KURLDrag::decode( e, urlList ) )
    {
        QString dropText = text();
        KURL::List::ConstIterator it;
        for( it = urlList.begin() ; it != urlList.end() ; ++it )
        {
            if(!dropText.isEmpty())
                dropText+=' ';

            dropText += (*it).prettyURL();
        }

        validateAndSet( dropText, dropText.length(), 0, 0);

        e->accept();
    }
    else
        QLineEdit::dropEvent(e);
}

bool KLineEdit::eventFilter( QObject* o, QEvent* ev )
{
    if( o == this )
    {
        KCursor::autoHideEventFilter( this, ev );
        if ( ev->type() == QEvent::AccelOverride )
        {
            QKeyEvent *e = static_cast<QKeyEvent *>( ev );
            if (overrideAccel (e))
            {
                e->accept();
                return true;
            }
        }
        else if( ev->type() == QEvent::KeyPress )
        {
            QKeyEvent *e = static_cast<QKeyEvent *>( ev );

            if( e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter )
            {
                bool trap = d->completionBox && d->completionBox->isVisible();

                bool stopEvent = trap || (d->grabReturnKeyEvents &&
                                          (e->state() == NoButton ||
                                           e->state() == Keypad));

                // Qt will emit returnPressed() itself if we return false
                if ( stopEvent )
                {
                  emit QLineEdit::returnPressed();
                  e->accept ();
                }

                emit returnPressed( displayText() );

                if ( trap )
                {
                    d->completionBox->hide();
                    deselect();
                    setCursorPosition(text().length());
                }

                // Eat the event if the user asked for it, or if a completionbox was visible
                return stopEvent;
            }
        }
    }
    return QLineEdit::eventFilter( o, ev );
}


void KLineEdit::setURLDropsEnabled(bool enable)
{
    d->handleURLDrops=enable;
}

bool KLineEdit::isURLDropsEnabled() const
{
    return d->handleURLDrops;
}

void KLineEdit::setTrapReturnKey( bool grab )
{
    d->grabReturnKeyEvents = grab;
}

bool KLineEdit::trapReturnKey() const
{
    return d->grabReturnKeyEvents;
}

void KLineEdit::setURL( const KURL& url )
{
    setText( url.prettyURL() );
}

void KLineEdit::makeCompletionBox()
{
    if ( d->completionBox )
        return;

    d->completionBox = new KCompletionBox( this, "completion box" );
    if ( handleSignals() )
    {
        connect( d->completionBox, SIGNAL(highlighted( const QString& )),
                 SLOT(setTextWorkaround( const QString& )) );
        connect( d->completionBox, SIGNAL(userCancelled( const QString& )),
                 SLOT(userCancelled( const QString& )) );

        connect( d->completionBox, SIGNAL( activated( const QString& )),
                 SIGNAL(completionBoxActivated( const QString& )) );
    }
}

void KLineEdit::userCancelled(const QString & cancelText)
{
    if ( completionMode() != KGlobalSettings::CompletionPopupAuto )
    {
      setText(cancelText);
    }
    else if (hasSelectedText() )
    {
      if (d->userSelection)
        deselect();
      else
      {
        d->autoSuggest=false;
        int start,end;
        getSelection(&start, &end);
        QString s=text().remove(start, end-start+1);
        validateAndSet(s,start,s.length(),s.length());
        d->autoSuggest=true;
      }
    }
}

bool KLineEdit::overrideAccel (const QKeyEvent* e)
{
    KShortcut scKey;

    KKey key( e );
    KeyBindingMap keys = getKeyBindings();

    if (keys[TextCompletion].isNull())
        scKey = KStdAccel::shortcut(KStdAccel::TextCompletion);
    else
        scKey = keys[TextCompletion];

    if (scKey.contains( key ))
        return true;

    if (keys[NextCompletionMatch].isNull())
        scKey = KStdAccel::shortcut(KStdAccel::NextCompletion);
    else
        scKey = keys[NextCompletionMatch];

    if (scKey.contains( key ))
        return true;

    if (keys[PrevCompletionMatch].isNull())
        scKey = KStdAccel::shortcut(KStdAccel::PrevCompletion);
    else
        scKey = keys[PrevCompletionMatch];

    if (scKey.contains( key ))
        return true;

    // Override all the text manupilation accelerators...
    if ( KStdAccel::copy().contains( key ) )
        return true;
    else if ( KStdAccel::paste().contains( key ) )
        return true;
    else if ( KStdAccel::cut().contains( key ) )
        return true;
    else if ( KStdAccel::undo().contains( key ) )
        return true;
    else if ( KStdAccel::redo().contains( key ) )
        return true;
    else if (KStdAccel::deleteWordBack().contains( key ))
        return true;
    else if (KStdAccel::deleteWordForward().contains( key ))
        return true;
    else if (KStdAccel::forwardWord().contains( key ))
        return true;
    else if (KStdAccel::backwardWord().contains( key ))
        return true;
    else if (KStdAccel::beginningOfLine().contains( key ))
        return true;
    else if (KStdAccel::endOfLine().contains( key ))
        return true;

    if (d->completionBox && d->completionBox->isVisible ())
    {
        int key = e->key();
        ButtonState state = e->state();
        if ((key == Key_Backtab || key == Key_Tab) &&
            (state == NoButton || (state & ShiftButton)))
        {
            return true;
        }
    }


    return false;
}

void KLineEdit::setCompletedItems( const QStringList& items )
{
    setCompletedItems( items, true );
}

void KLineEdit::setCompletedItems( const QStringList& items, bool autoSuggest )
{
    QString txt = text();

    if ( !items.isEmpty() &&
         !(items.count() == 1 && txt == items.first()) )
    {
        if ( !d->completionBox )
            makeCompletionBox();

        if ( !txt.isEmpty() )
            d->completionBox->setCancelledText( txt );

        d->completionBox->setItems( items );
        d->completionBox->popup();

        if ( d->autoSuggest && autoSuggest )
        {
            int index = items.first().find( txt );
            QString newText = items.first().mid( index );
            setUserSelection(false);
            setCompletedText(newText,true);
        }
    }
    else
    {
        if ( d->completionBox && d->completionBox->isVisible() )
            d->completionBox->hide();
    }
}

KCompletionBox * KLineEdit::completionBox( bool create )
{
    if ( create )
        makeCompletionBox();

    return d->completionBox;
}

void KLineEdit::setCompletionObject( KCompletion* comp, bool hsig )
{
    KCompletion *oldComp = compObj();
    if ( oldComp && handleSignals() )
        disconnect( oldComp, SIGNAL( matches( const QStringList& )),
                    this, SLOT( setCompletedItems( const QStringList& )));

    if ( comp && hsig )
      connect( comp, SIGNAL( matches( const QStringList& )),
               this, SLOT( setCompletedItems( const QStringList& )));

    KCompletionBase::setCompletionObject( comp, hsig );
}

// QWidget::create() turns off mouse-Tracking which would break auto-hiding
void KLineEdit::create( WId id, bool initializeWindow, bool destroyOldWindow )
{
    QLineEdit::create( id, initializeWindow, destroyOldWindow );
    KCursor::setAutoHideCursor( this, true, true );
}

void KLineEdit::setUserSelection(bool userSelection)
{
    QPalette p = palette();

    if (userSelection)
    {
        p.setColor(QColorGroup::Highlight, d->previousHighlightColor);
        p.setColor(QColorGroup::HighlightedText, d->previousHighlightedTextColor);
    }
    else
    {
        QColor color=p.color(QPalette::Disabled, QColorGroup::Text);
        p.setColor(QColorGroup::HighlightedText, color);
        color=p.color(QPalette::Active, QColorGroup::Base);
        p.setColor(QColorGroup::Highlight, color);
    }

    d->userSelection=userSelection;
    setPalette(p);
}

void KLineEdit::slotRestoreSelectionColors()
{
    if (d->disableRestoreSelection)
      return;

    setUserSelection(true);
}

void KLineEdit::clear()
{
    setText( QString::null );
}

void KLineEdit::setTextWorkaround( const QString& text )
{
    setText( text );
    end( false ); // force cursor at end
}

QString KLineEdit::originalText() const
{
    if ( d->enableSqueezedText && isReadOnly() )
        return d->squeezedText;

    return text();
}

void KLineEdit::virtual_hook( int id, void* data )
{ KCompletionBase::virtual_hook( id, data ); }

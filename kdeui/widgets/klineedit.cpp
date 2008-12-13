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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "klineedit.h"
#include "klineedit_p.h"
#include "kdeuiwidgetsproxystyle_p.h"

#include <kaction.h>
#include <kapplication.h>
#include <kauthorized.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kcompletionbox.h>
#include <kicontheme.h>
#include <kicon.h>
#include <klocale.h>
#include <kmenu.h>
#include <kstandardaction.h>
#include <kstandardshortcut.h>

#include <QtCore/QTimer>
#include <QtGui/QClipboard>
#include <QtGui/QStyleOption>
#include <QtGui/QToolTip>

class KLineEditPrivate
{
public:
    KLineEditPrivate(KLineEdit* qq)
        : q(qq)
    {
        completionBox = 0L;
        handleURLDrops = true;
        grabReturnKeyEvents = false;

        userSelection = true;
        autoSuggest = false;
        disableRestoreSelection = false;
        enableSqueezedText = false;

        drawClickMsg = false;
        enableClickMsg = false;
        threeStars = false;
        if ( !initialized )
        {
            KConfigGroup config( KGlobal::config(), "General" );
            backspacePerformsCompletion = config.readEntry("Backspace performs completion", false);

            initialized = true;
        }

        clearButton = 0;
        clickInClear = false;
        wideEnoughForClear = true;
        overlap = 0;
    }

    ~KLineEditPrivate()
    {
// causes a weird crash in KWord at least, so let Qt delete it for us.
//        delete completionBox;
    }

    void _k_slotSettingsChanged(int category)
    {
        Q_UNUSED(category);

        if (clearButton) {
            clearButton->setAnimationsEnabled(KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects);
        }
    }

    void doCompletion(const QString& txt);

    /**
     * Checks whether we should/should not consume a key used as a shortcut.
     * This makes it possible to handle shortcuts in the focused widget before any
     * window-global QAction is triggered.
     */
    bool overrideShortcut(const QKeyEvent* e);

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
    QPalette::ColorRole bgRole;
    QString squeezedText;

    QString clickMessage;
    bool enableClickMsg:1;
    bool drawClickMsg:1;
    bool threeStars:1;

    bool possibleTripleClick :1;  // set in mousePressEvent, deleted in tripleClickTimeout

    bool clickInClear:1;
    bool wideEnoughForClear:1;
    KLineEditButton *clearButton;

    KCompletionBox *completionBox;

    int overlap;

    QAction *noCompletionAction, *shellCompletionAction, *autoCompletionAction, *popupCompletionAction, *shortAutoCompletionAction, *popupAutoCompletionAction, *defaultAction;

    QMap<KGlobalSettings::Completion, bool> disableCompletionMap;
    KLineEdit* q;
};

// FIXME: Go back to using StyleSheets instead of a proxy style
// once Qt has been fixed not to mess with widget font when
// using StyleSheets
class KLineEditStyle : public KdeUiProxyStyle
{
public:
  KLineEditStyle(KLineEdit *parent, KLineEditPrivate *lineEditPrivate)
    : KdeUiProxyStyle(parent), lineEditPrivate(lineEditPrivate) {}

  QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const;

  KLineEditPrivate* lineEditPrivate;
};

QRect KLineEditStyle::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
  if (element == SE_LineEditContents)
  {
    QRect rect = style()->subElementRect(SE_LineEditContents, option, widget);

    int overlap = lineEditPrivate->overlap;
    if (option->direction == Qt::LeftToRight) return rect.adjusted(0, 0, -overlap, 0);
    else return rect.adjusted(overlap, 0, 0, 0);
  }

  return KdeUiProxyStyle::subElementRect(element, option, widget);
}

bool KLineEditPrivate::backspacePerformsCompletion = false;
bool KLineEditPrivate::initialized = false;


KLineEdit::KLineEdit( const QString &string, QWidget *parent )
    : QLineEdit( string, parent ), d(new KLineEditPrivate(this))
{
    init();
}

KLineEdit::KLineEdit( QWidget *parent )
    : QLineEdit( parent ), d(new KLineEditPrivate(this))
{
    init();
}


KLineEdit::~KLineEdit ()
{
    delete d;
}

void KLineEdit::init()
{
    d->possibleTripleClick = false;
    d->bgRole = backgroundRole();

    // Enable the context menu by default.
    QLineEdit::setContextMenuPolicy( Qt::DefaultContextMenu );
    KCursor::setAutoHideCursor( this, true, true );

    KGlobalSettings::Completion mode = completionMode();
    d->autoSuggest = (mode == KGlobalSettings::CompletionMan ||
                      mode == KGlobalSettings::CompletionPopupAuto ||
                      mode == KGlobalSettings::CompletionAuto);
    connect( this, SIGNAL(selectionChanged()), this, SLOT(slotRestoreSelectionColors()));

    connect(KGlobalSettings::self(), SIGNAL(settingsChanged(int)), this, SLOT(_k_slotSettingsChanged(int)));

    QPalette p = palette();
    if ( !d->previousHighlightedTextColor.isValid() )
      d->previousHighlightedTextColor=p.color(QPalette::Normal,QPalette::HighlightedText);
    if ( !d->previousHighlightColor.isValid() )
      d->previousHighlightColor=p.color(QPalette::Normal,QPalette::Highlight);

    QStyle *lineEditStyle = new KLineEditStyle(this, d);
    lineEditStyle->setParent(this);
    setStyle(lineEditStyle);
}

QString KLineEdit::clickMessage() const
{
    return d->clickMessage;
}

void KLineEdit::setClearButtonShown(bool show)
{
    if (show) {
        if (d->clearButton) {
            return;
        }

        d->clearButton = new KLineEditButton(this);
        d->clearButton->setCursor( Qt::ArrowCursor );
        d->clearButton->setToolTip( i18nc( "@action:button Clear current text in the line edit", "Clear text" ) );

        updateClearButtonIcon(text());
        updateClearButton();
        connect(this, SIGNAL(textChanged(QString)), this, SLOT(updateClearButtonIcon(QString)));
    } else {
        disconnect(this, SIGNAL(textChanged(QString)), this, SLOT(updateClearButtonIcon(QString)));
        delete d->clearButton;
        d->clearButton = 0;
        d->clickInClear = false;
        d->overlap = 0;
    }
}

bool KLineEdit::isClearButtonShown() const
{
    return d->clearButton != 0;
}

QSize KLineEdit::clearButtonUsedSize() const
{
    QSize s;
    if (d->clearButton) {
        const int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, 0, this);
        s = d->clearButton->sizeHint();
        s.rwidth() += frameWidth;
    }
    return s;
}

void KLineEdit::updateClearButtonIcon(const QString& text)
{
    if (!d->clearButton || isReadOnly()) {
        return;
    }

    int clearButtonState = KIconLoader::DefaultState;

    if (d->wideEnoughForClear && text.length() > 0) {
        d->clearButton->animateVisible(true);
    } else {
        d->clearButton->animateVisible(false);
    }

    if (!d->clearButton->pixmap().isNull()) {
        return;
    }

    if (layoutDirection() == Qt::LeftToRight) {
        d->clearButton->setPixmap(SmallIcon("edit-clear-locationbar-rtl", 0, clearButtonState));
    } else {
        d->clearButton->setPixmap(SmallIcon("edit-clear-locationbar-ltr", 0, clearButtonState));
    }

    d->clearButton->setVisible(text.length());
}

void KLineEdit::updateClearButton()
{
    if (!d->clearButton || isReadOnly()) {
        return;
    }

    const QSize geom = size();
    const int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth,0,this);
    const int buttonWidth = d->clearButton->sizeHint().width();
    const QSize newButtonSize(buttonWidth, geom.height());
    const QFontMetrics fm(font());
    const int em = fm.width("m");

    // make sure we have enough room for the clear button
    // no point in showing it if we can't also see a few characters as well
    const bool wideEnough = geom.width() > 4 * em + buttonWidth + frameWidth;

    if (newButtonSize != d->clearButton->size()) {
        d->clearButton->resize(newButtonSize);
        d->overlap = wideEnough ? buttonWidth + frameWidth : 0;
    }

    if (layoutDirection() == Qt::LeftToRight ) {
        d->clearButton->move(geom.width() - frameWidth - buttonWidth - 1, 0);
    } else {
        d->clearButton->move(frameWidth + 1, 0);
    }

    if (wideEnough != d->wideEnoughForClear) {
        // we may (or may not) have been showing the button, but now our
        // positiong on that matter has shifted, so let's ensure that it
        // is properly visible (or not)
        d->wideEnoughForClear = wideEnough;
        updateClearButtonIcon(text());
    }
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

    if ( kapp && !KAuthorized::authorize("lineedit_text_completion") )
        mode = KGlobalSettings::CompletionNone;

    if ( mode == KGlobalSettings::CompletionPopupAuto ||
         mode == KGlobalSettings::CompletionAuto ||
         mode == KGlobalSettings::CompletionMan )
        d->autoSuggest = true;
    else
        d->autoSuggest = false;

    KCompletionBase::setCompletionMode( mode );
}

void KLineEdit::setCompletionModeDisabled( KGlobalSettings::Completion mode, bool disable )
{
  d->disableCompletionMap[ mode ] = disable;
}

void KLineEdit::setCompletedText( const QString& t, bool marked )
{
    if ( !d->autoSuggest )
      return;

    QString txt = text();

    if ( t != txt )
    {
        int start = marked ? txt.length() : t.length();
        setText(t);
        setSelection(start, t.length());
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
          input = comp->previousMatch();
       else
          input = comp->nextMatch();

       // Skip rotation if previous/next match is null or the same text
       if ( input.isEmpty() || input == displayText() )
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

    const QString match = comp->makeCompletion( text );

    if ( mode == KGlobalSettings::CompletionPopup ||
         mode == KGlobalSettings::CompletionPopupAuto )
    {
        if ( match.isEmpty() )
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
        if ( match.isEmpty() || match == text )
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
        d->bgRole = backgroundRole();
        setBackgroundRole(QPalette::Window);
        if (d->enableSqueezedText && d->squeezedText.isEmpty())
        {
            d->squeezedText = text();
            setSqueezedText();
        }

        if (d->clearButton) {
            d->clearButton->animateVisible(false);
            d->overlap = 0;
        }
    }
    else
    {
        if (!d->squeezedText.isEmpty())
        {
           setText(d->squeezedText);
           d->squeezedText.clear();
        }
        setBackgroundRole(d->bgRole);

        if (d->clearButton && !text().isEmpty()) {
            int buttonWidth = d->clearButton->sizeHint().width();
            d->clearButton->animateVisible(true);
            d->overlap = buttonWidth;
        }
    }
}

void KLineEdit::setSqueezedText( const QString &text)
{
    setSqueezedTextEnabled(true);
    setText(text);
}

void KLineEdit::setSqueezedTextEnabled( bool enable )
{
    d->enableSqueezedText = enable;
}

bool KLineEdit::isSqueezedTextEnabled() const
{
    return d->enableSqueezedText;
}

void KLineEdit::setText( const QString& text )
{
    if( d->enableClickMsg )
    {
          d->drawClickMsg = text.isEmpty();
          update();
    }
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
    int labelWidth = size().width() - 2*style()->pixelMetric(QStyle::PM_DefaultFrameWidth) - 2;
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

          setToolTip( fullText );

    }
    else
    {
      QLineEdit::setText(fullText);

      this->setToolTip( "" );
      QToolTip::showText(pos(), QString()); // hide
    }

    setCursorPosition(0);
}

void KLineEdit::copy() const
{
    if( !copySqueezedText(true))
        QLineEdit::copy();
}

bool KLineEdit::copySqueezedText(bool clipboard) const
{
   if (!d->squeezedText.isEmpty() && d->squeezedStart)
   {
      KLineEdit *that = const_cast<KLineEdit *>(this);
      if (!that->hasSelectedText())
         return false;
      int start = selectionStart(), end = start + selectedText().length();
      if (start >= d->squeezedStart+3)
         start = start - 3 - d->squeezedStart + d->squeezedEnd;
      else if (start > d->squeezedStart)
         start = d->squeezedStart;
      if (end >= d->squeezedStart+3)
         end = end - 3 - d->squeezedStart + d->squeezedEnd;
      else if (end > d->squeezedStart)
         end = d->squeezedEnd;
      if (start == end)
         return false;
      QString t = d->squeezedText;
      t = t.mid(start, end - start);
      disconnect( QApplication::clipboard(), SIGNAL(selectionChanged()), this, 0);
      QApplication::clipboard()->setText( t, clipboard ? QClipboard::Clipboard : QClipboard::Selection );
      connect( QApplication::clipboard(), SIGNAL(selectionChanged()), this,
               SLOT(_q_clipboardChanged()) );
      return true;
   }
   return false;
}

void KLineEdit::resizeEvent( QResizeEvent * ev )
{
    if (!d->squeezedText.isEmpty())
        setSqueezedText();

    updateClearButton();
    QLineEdit::resizeEvent(ev);
}

void KLineEdit::keyPressEvent( QKeyEvent *e )
{
    const int key = e->key() | e->modifiers();

    if ( KStandardShortcut::copy().contains( key ) )
    {
        copy();
        return;
    }
    else if ( KStandardShortcut::paste().contains( key ) )
    {
        paste();
        return;
    }
    else if ( KStandardShortcut::pasteSelection().contains( key ) )
    {
        QString text = QApplication::clipboard()->text( QClipboard::Selection);
        insert( text );
        deselect();
        return;
    }

    else if ( KStandardShortcut::cut().contains( key ) )
    {
        cut();
        return;
    }
    else if ( KStandardShortcut::undo().contains( key ) )
    {
        undo();
        return;
    }
    else if ( KStandardShortcut::redo().contains( key ) )
    {
        redo();
        return;
    }
    else if ( KStandardShortcut::deleteWordBack().contains( key ) )
    {
        cursorWordBackward(true);
        if ( hasSelectedText() )
            del();

        e->accept();
        return;
    }
    else if ( KStandardShortcut::deleteWordForward().contains( key ) )
    {
        // Workaround for QT bug where
        cursorWordForward(true);
        if ( hasSelectedText() )
            del();

        e->accept();
        return;
    }
    else if ( KStandardShortcut::backwardWord().contains( key ) )
    {
      cursorWordBackward(false);
      e->accept();
      return;
    }
    else if ( KStandardShortcut::forwardWord().contains( key ) )
    {
      cursorWordForward(false);
      e->accept();
      return;
    }
    else if ( KStandardShortcut::beginningOfLine().contains( key ) )
    {
      home(false);
      e->accept();
      return;
    }
    else if ( KStandardShortcut::endOfLine().contains( key ) )
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
        const KeyBindingMap keys = getKeyBindings();
        KGlobalSettings::Completion mode = completionMode();
        bool noModifier = (e->modifiers() == Qt::NoButton ||
                           e->modifiers() == Qt::ShiftModifier ||
                           e->modifiers() == Qt::KeypadModifier);

        if ( (mode == KGlobalSettings::CompletionAuto ||
              mode == KGlobalSettings::CompletionPopupAuto ||
              mode == KGlobalSettings::CompletionMan) && noModifier )
        {
            if ( !d->userSelection && hasSelectedText() &&
                 ( e->key() == Qt::Key_Right || e->key() == Qt::Key_Left ) &&
                 e->modifiers()==Qt::NoButton )
            {
                QString old_txt = text();
                d->disableRestoreSelection = true;
                int start = selectionStart();

                deselect();
                QLineEdit::keyPressEvent ( e );
                int cPosition=cursorPosition();
                setText(old_txt);
                setCursorPosition(cPosition);
                if (e->key() ==Qt::Key_Right && cPosition > start )
                    setSelection(cPosition, old_txt.length());
                else
                    setSelection(start, old_txt.length());

                d->disableRestoreSelection = false;
                return;
            }

            if ( e->key() == Qt::Key_Escape )
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
                e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete ) )
            {
                bool hasUserSelection=d->userSelection;
                bool hadSelection=hasSelectedText();

                bool cursorNotAtEnd=false;

                int start = selectionStart();
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
                    if ( e->key() == Qt::Key_Backspace )
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

                    if (e->key() == Qt::Key_Delete )
                        d->autoSuggest=false;

                    d->doCompletion(txt);

                    if(  (e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete) )
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

            int start = selectionStart();
            int cPos = cursorPosition();
            QString keycode = e->text();

            // When moving the cursor, we want to keep the autocompletion as an
            // autocompletion, so we want to process events at the cursor position
            // as if there was no selection. After processing the key event, we
            // can set the new autocompletion again.
            if (hadSelection && !hasUserSelection && start>cPos &&
               ( (!keycode.isEmpty() && keycode.unicode()->isPrint()) ||
                 e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete ) )
            {
                del();
                setCursorPosition(cPos);
                cursorNotAtEnd=true;
            }

            int selectedLength=selectedText().length();

            d->disableRestoreSelection = true;
            QLineEdit::keyPressEvent ( e );
            d->disableRestoreSelection = false;

            if (( selectedLength != selectedText().length() ) && !hasUserSelection )
                slotRestoreSelectionColors(); // and set userSelection to true

            QString txt = text();
            int len = txt.length();
            if ( ( txt != old_txt || txt != e->text() ) && len/* && ( cursorPosition() == len || force )*/ &&
                 ( (!keycode.isEmpty() && keycode.unicode()->isPrint()) ||
                   e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete) )
            {
                if ( e->key() == Qt::Key_Backspace )
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

                if (e->key() == Qt::Key_Delete )
                    d->autoSuggest=false;

                if ( d->completionBox )
                  d->completionBox->setCancelledText( txt );

                d->doCompletion(txt);

                if ( (e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete ) &&
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
            if ( keys[TextCompletion].isEmpty() )
                cut = KStandardShortcut::shortcut(KStandardShortcut::TextCompletion);
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
                    d->doCompletion(txt);
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
            if ( keys[PrevCompletionMatch].isEmpty() )
                cut = KStandardShortcut::shortcut(KStandardShortcut::PrevCompletion);
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
            if ( keys[NextCompletionMatch].isEmpty() )
                cut = KStandardShortcut::shortcut(KStandardShortcut::NextCompletion);
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
            if ( keys[SubstringCompletion].isEmpty() )
                cut = KStandardShortcut::shortcut(KStandardShortcut::SubstringCompletion);
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

    int selectedLength = selectedText().length();

    // Let QLineEdit handle any other keys events.
    QLineEdit::keyPressEvent ( e );

    if ( selectedLength != selectedText().length() )
        slotRestoreSelectionColors(); // and set userSelection to true
}

void KLineEdit::mouseDoubleClickEvent( QMouseEvent* e )
{
    if ( e->button() == Qt::LeftButton  )
    {
        d->possibleTripleClick=true;
        QTimer::singleShot( QApplication::doubleClickInterval(),this,
                            SLOT(tripleClickTimeout()) );
    }
    QLineEdit::mouseDoubleClickEvent( e );
}

void KLineEdit::mousePressEvent( QMouseEvent* e )
{
    if  ( (e->button() == Qt::LeftButton ||
           e->button() == Qt::MidButton ) &&
          d->clearButton ) {
        d->clickInClear = d->clearButton->underMouse();

        if ( d->clickInClear ) {
            d->possibleTripleClick = false;
        }
    }

    if ( e->button() == Qt::LeftButton && d->possibleTripleClick ) {
        selectAll();
        e->accept();
        return;
    }

    QLineEdit::mousePressEvent( e );
}

void KLineEdit::mouseReleaseEvent( QMouseEvent* e )
{
    if ( d->clickInClear ) {
        if ( d->clearButton->underMouse() ) {
            QString newText;
            if ( e->button() == Qt::MidButton ) {
                newText = QApplication::clipboard()->text( QClipboard::Selection );
                setText( newText );
            } else {
                setSelection(0, text().size());
                del();
                emit clearButtonClicked();
            }
            emit textChanged( newText );
        }

        d->clickInClear = false;
        e->accept();
        return;
    }

    QLineEdit::mouseReleaseEvent( e );

   if (QApplication::clipboard()->supportsSelection() ) {
       if ( e->button() == Qt::LeftButton ) {
            // Fix copying of squeezed text if needed
            copySqueezedText( false );
       }
   }
}

void KLineEdit::tripleClickTimeout()
{
    d->possibleTripleClick=false;
}

QMenu* KLineEdit::createStandardContextMenu()
{
    QMenu *popup = QLineEdit::createStandardContextMenu();

    if( !isReadOnly() )
    {
        // FIXME: This code depends on Qt's action ordering.
        QList<QAction *> actionList = popup->actions();
        enum { UndoAct, RedoAct, Separator1, CutAct, CopyAct, PasteAct, DeleteAct, ClearAct,
               Separator2, SelectAllAct, NCountActs };
        QAction *separatorAction = 0L;
        // separator we want is right after Delete right now.
        int idx = actionList.indexOf( actionList[DeleteAct] ) + 1;
        if ( idx < actionList.count() )
            separatorAction = actionList.at( idx );
        if ( separatorAction )
        {
            KAction *clearAllAction = KStandardAction::clear( this, SLOT( clear() ), this) ;
            if ( text().isEmpty() )
                clearAllAction->setEnabled( false );
            popup->insertAction( separatorAction, clearAllAction );
        }
    }

    KIconTheme::assignIconsToContextMenu( KIconTheme::TextEditor, popup->actions () );

    // If a completion object is present and the input
    // widget is not read-only, show the Text Completion
    // menu item.
    if ( compObj() && !isReadOnly() && KAuthorized::authorize("lineedit_text_completion") )
    {
        QMenu *subMenu = popup->addMenu( KIcon("text-completion"), i18nc("@title:menu", "Text Completion") );
        connect( subMenu, SIGNAL( triggered ( QAction* ) ),
                 this, SLOT( completionMenuActivated( QAction* ) ) );

        popup->addSeparator();

        QActionGroup* ag = new QActionGroup( this );
        d->noCompletionAction = ag->addAction( i18nc("@item:inmenu Text Completion", "None"));
        d->shellCompletionAction = ag->addAction( i18nc("@item:inmenu Text Completion", "Manual") );
        d->autoCompletionAction = ag->addAction( i18nc("@item:inmenu Text Completion", "Automatic") );
        d->popupCompletionAction = ag->addAction( i18nc("@item:inmenu Text Completion", "Dropdown List") );
        d->shortAutoCompletionAction = ag->addAction( i18nc("@item:inmenu Text Completion", "Short Automatic") );
        d->popupAutoCompletionAction = ag->addAction( i18nc("@item:inmenu Text Completion", "Dropdown List && Automatic"));
        subMenu->addActions( ag->actions() );

        //subMenu->setAccel( KStandardShortcut::completion(), ShellCompletion );

        d->shellCompletionAction->setCheckable( true );
        d->noCompletionAction->setCheckable( true );
        d->popupCompletionAction->setCheckable( true );
        d->autoCompletionAction->setCheckable( true );
        d->shortAutoCompletionAction->setCheckable( true );
        d->popupAutoCompletionAction->setCheckable( true );

        d->shellCompletionAction->setEnabled( !d->disableCompletionMap[ KGlobalSettings::CompletionShell ] );
        d->noCompletionAction->setEnabled( !d->disableCompletionMap[ KGlobalSettings::CompletionNone ] );
        d->popupCompletionAction->setEnabled( !d->disableCompletionMap[ KGlobalSettings::CompletionPopup ] );
        d->autoCompletionAction->setEnabled( !d->disableCompletionMap[ KGlobalSettings::CompletionAuto ] );
        d->shortAutoCompletionAction->setEnabled( !d->disableCompletionMap[ KGlobalSettings::CompletionMan ] );
        d->popupAutoCompletionAction->setEnabled( !d->disableCompletionMap[ KGlobalSettings::CompletionPopupAuto ] );

        KGlobalSettings::Completion mode = completionMode();
        d->noCompletionAction->setChecked( mode == KGlobalSettings::CompletionNone );
        d->shellCompletionAction->setChecked( mode == KGlobalSettings::CompletionShell );
        d->popupCompletionAction->setChecked( mode == KGlobalSettings::CompletionPopup );
        d->autoCompletionAction->setChecked(  mode == KGlobalSettings::CompletionAuto );
        d->shortAutoCompletionAction->setChecked( mode == KGlobalSettings::CompletionMan );
        d->popupAutoCompletionAction->setChecked( mode == KGlobalSettings::CompletionPopupAuto );
        if ( mode != KGlobalSettings::completionMode() )
        {
            subMenu->addSeparator();
            d->defaultAction = subMenu->addAction( i18nc("@item:inmenu Text Completion", "Default") );
        }
    }

    return popup;
}

void KLineEdit::contextMenuEvent( QContextMenuEvent *e )
{
    if ( QLineEdit::contextMenuPolicy() != Qt::DefaultContextMenu )
      return;
    QMenu *popup = createStandardContextMenu();

    // ### do we really need this?  Yes, Please do not remove!  This
    // allows applications to extend the popup menu without having to
    // inherit from this class! (DA)
    emit aboutToShowContextMenu( popup );

    popup->exec(e->globalPos());
    delete popup;
}

void KLineEdit::completionMenuActivated( QAction  *act)
{
    KGlobalSettings::Completion oldMode = completionMode();

    if( act == d->noCompletionAction )
    {
        setCompletionMode( KGlobalSettings::CompletionNone );
    }
    else if( act ==  d->shellCompletionAction)
    {
        setCompletionMode( KGlobalSettings::CompletionShell );
    }
    else if( act == d->autoCompletionAction)
    {
        setCompletionMode( KGlobalSettings::CompletionAuto );
    }
    else if( act == d->popupCompletionAction)
    {
        setCompletionMode( KGlobalSettings::CompletionPopup );
    }
    else if( act == d->shortAutoCompletionAction)
    {
        setCompletionMode( KGlobalSettings::CompletionMan );
    }
    else if( act == d->popupAutoCompletionAction)
    {
        setCompletionMode( KGlobalSettings::CompletionPopupAuto );
    }
    else if( act == d->defaultAction )
    {
        setCompletionMode( KGlobalSettings::completionMode() );
    }
    else
        return;

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
    if( d->handleURLDrops )
    {
        const KUrl::List urlList = KUrl::List::fromMimeData( e->mimeData() );
        if ( !urlList.isEmpty() )
        {
            QString dropText = text();
            KUrl::List::ConstIterator it;
            for( it = urlList.begin() ; it != urlList.end() ; ++it )
            {
                if(!dropText.isEmpty())
                    dropText+=' ';

                dropText += (*it).prettyUrl();
            }

            setText(dropText);
            setCursorPosition(dropText.length());

            e->accept();
            return;
        }
    }
    QLineEdit::dropEvent(e);
}

bool KLineEdit::event( QEvent* ev )
{
    KCursor::autoHideEventFilter( this, ev );
    if ( ev->type() == QEvent::ShortcutOverride )
    {
        QKeyEvent *e = static_cast<QKeyEvent *>( ev );
        if (d->overrideShortcut(e)) {
            ev->accept();
        }
    }
    else if( ev->type() == QEvent::KeyPress )
    {
        // Hmm -- all this could be done in keyPressEvent too...

        QKeyEvent *e = static_cast<QKeyEvent *>( ev );

        if( e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter )
        {
            bool trap = d->completionBox && d->completionBox->isVisible();

            bool stopEvent = trap || (d->grabReturnKeyEvents &&
                                      (e->modifiers() == Qt::NoButton ||
                                       e->modifiers() == Qt::KeypadModifier));

            // Qt will emit returnPressed() itself if we return false
            if ( stopEvent )
            {
                emit QLineEdit::returnPressed();
                e->accept();
            }

            emit returnPressed( displayText() );

            if ( trap )
            {
                d->completionBox->hide();
                deselect();
                setCursorPosition(text().length());
            }

            // Eat the event if the user asked for it, or if a completionbox was visible
            if (stopEvent)
                return true;
        } else if (e->key() == Qt::Key_Tab && e->modifiers() == Qt::NoButton ) {
            // #65877: Key_Tab should complete using the first (or selected) item, and then offer completions again
            const bool tabHandling = d->completionBox && d->completionBox->isVisible() && d->completionBox->count() > 0;
            if (tabHandling) {
                QListWidgetItem* currentItem = d->completionBox->currentItem();
                const QString txt = currentItem ? currentItem->text() : d->completionBox->item(0)->text();
                setTextWorkaround(txt);
                d->doCompletion(txt);
                e->accept();
                return true;
            }
        }
    }
    return QLineEdit::event( ev );
}


void KLineEdit::setUrlDropsEnabled(bool enable)
{
    d->handleURLDrops=enable;
}

bool KLineEdit::urlDropsEnabled() const
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

void KLineEdit::setUrl( const KUrl& url )
{
    setText( url.prettyUrl() );
}

void KLineEdit::setCompletionBox( KCompletionBox *box )
{
    if ( d->completionBox )
        return;

    d->completionBox = box;
    if ( handleSignals() )
    {
        connect( d->completionBox, SIGNAL(currentTextChanged( const QString& )),
                 SLOT(setTextWorkaround( const QString& )) );
        connect( d->completionBox, SIGNAL(userCancelled( const QString& )),
                 SLOT(userCancelled( const QString& )) );

        // TODO: we need our own slot, and to call setModified(true) if Qt4 has that.
        connect( d->completionBox, SIGNAL( activated( const QString& )),
                 SIGNAL(completionBoxActivated( const QString& )) );
    }
}

void KLineEdit::userCancelled(const QString & cancelText)
{
    if ( completionMode() != KGlobalSettings::CompletionPopupAuto )
    {
      // TODO: this sets modified==false. But maybe it was true before...
      setText(cancelText);
    }
    else if (hasSelectedText() )
    {
      if (d->userSelection)
        deselect();
      else
      {
        d->autoSuggest=false;
        int start = selectionStart() ;
        QString s=text().remove(selectionStart(), selectedText().length());
        setText(s);
        setCursorPosition(start);
        d->autoSuggest=true;
      }
    }
}

bool KLineEditPrivate::overrideShortcut(const QKeyEvent* e)
{
    KShortcut scKey;

    int key = e->key() | e->modifiers();
    const KLineEdit::KeyBindingMap keys = q->getKeyBindings();

    if (keys[KLineEdit::TextCompletion].isEmpty())
        scKey = KStandardShortcut::shortcut(KStandardShortcut::TextCompletion);
    else
        scKey = keys[KLineEdit::TextCompletion];

    if (scKey.contains( key ))
        return true;

    if (keys[KLineEdit::NextCompletionMatch].isEmpty())
        scKey = KStandardShortcut::shortcut(KStandardShortcut::NextCompletion);
    else
        scKey = keys[KLineEdit::NextCompletionMatch];

    if (scKey.contains( key ))
        return true;

    if (keys[KLineEdit::PrevCompletionMatch].isEmpty())
        scKey = KStandardShortcut::shortcut(KStandardShortcut::PrevCompletion);
    else
        scKey = keys[KLineEdit::PrevCompletionMatch];

    if (scKey.contains( key ))
        return true;

    // Override all the text manupilation accelerators...
    if ( KStandardShortcut::copy().contains( key ) )
        return true;
    else if ( KStandardShortcut::paste().contains( key ) )
        return true;
    else if ( KStandardShortcut::cut().contains( key ) )
        return true;
    else if ( KStandardShortcut::undo().contains( key ) )
        return true;
    else if ( KStandardShortcut::redo().contains( key ) )
        return true;
    else if (KStandardShortcut::deleteWordBack().contains( key ))
        return true;
    else if (KStandardShortcut::deleteWordForward().contains( key ))
        return true;
    else if (KStandardShortcut::forwardWord().contains( key ))
        return true;
    else if (KStandardShortcut::backwardWord().contains( key ))
        return true;
    else if (KStandardShortcut::beginningOfLine().contains( key ))
        return true;
    else if (KStandardShortcut::endOfLine().contains( key ))
        return true;

    if (completionBox && completionBox->isVisible ())
    {
        const int key = e->key();
        Qt::KeyboardModifiers modifiers = e->modifiers();
        if ((key == Qt::Key_Backtab || key == Qt::Key_Tab) &&
            (modifiers == Qt::NoModifier || (modifiers & Qt::ShiftModifier)))
        {
            return true;
        }
    }


    return false;
}

void KLineEdit::setCompletedItems( const QStringList& items, bool autoSuggest )
{
    QString txt;
    if ( d->completionBox && d->completionBox->isVisible() ) {
        // The popup is visible already - do the matching on the initial string,
        // not on the currently selected one.
        txt = completionBox()->cancelledText();
    } else {
        txt = text();
    }

    if ( !items.isEmpty() &&
         !(items.count() == 1 && txt == items.first()) )
    {
        // create completion box if non-existent
        completionBox();

        if ( d->completionBox->isVisible() )
        {
            QListWidgetItem* currentItem = d->completionBox->currentItem();

            bool wasSelected = false;
            QString currentSelection;

            if ( currentItem != 0 ) {
                wasSelected = currentItem->isSelected();
                currentSelection = currentItem->text();
            }

            d->completionBox->setItems( items );

            const QList<QListWidgetItem*> matchedItems = d->completionBox->findItems( currentSelection , Qt::MatchExactly);
            QListWidgetItem* matchedItem = matchedItems.isEmpty() ? 0 : matchedItems.first();

            // If no item is selected, that means the listbox hasn't been manipulated by the user yet,
            // because it's not possible otherwise to have no selected item. In such case make
            // always the first item current and unselected, so that the current item doesn't jump.
            if( !matchedItem || !wasSelected )
            {
                wasSelected = false;
                matchedItem = d->completionBox->item( 0 );
            }
            if ( matchedItem )
            {
                bool blocked = d->completionBox->blockSignals( true );
                d->completionBox->setCurrentItem( matchedItem );
                matchedItem->setSelected(wasSelected);
                d->completionBox->blockSignals( blocked );
            }
        }
        else // completion box not visible yet -> show it
        {
            if ( !txt.isEmpty() )
                d->completionBox->setCancelledText( txt );
            d->completionBox->setItems( items );
            d->completionBox->popup();
        }

        if ( d->autoSuggest && autoSuggest )
        {
            int index = items.first().indexOf( txt );
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
    if ( create && !d->completionBox ) {
        setCompletionBox( new KCompletionBox( this ) );
        d->completionBox->setObjectName("completion box");
        d->completionBox->setFont(font());
    }

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
        p.setColor(QPalette::Highlight, d->previousHighlightColor);
        p.setColor(QPalette::HighlightedText, d->previousHighlightedTextColor);
    }
    else
    {
        QColor color=p.color(QPalette::Disabled, QPalette::Text);
        p.setColor(QPalette::HighlightedText, color);
        color=p.color(QPalette::Active, QPalette::Base);
        p.setColor(QPalette::Highlight, color);
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
    setText( QString() );
}

void KLineEdit::setTextWorkaround( const QString& text )
{
    if (!text.isEmpty())
    {
        setText( text );
        end( false ); // force cursor at end
    }
}

QString KLineEdit::originalText() const
{
    if ( d->enableSqueezedText && isReadOnly() )
        return d->squeezedText;

    return text();
}

bool KLineEdit::autoSuggest() const
{
    return d->autoSuggest;
}

void KLineEdit::paintEvent( QPaintEvent *ev )
{
    if (echoMode() == Password && d->threeStars) {
        const QString oldText = text();
        const bool isModifiedState = isModified(); // save modified state because setText resets it
        setText(oldText + oldText + oldText);
        QLineEdit::paintEvent(ev);
        setText(oldText);
        setModified(isModifiedState);
        return;
    }

    QLineEdit::paintEvent( ev );

    if (d->enableClickMsg && d->drawClickMsg && !hasFocus() && text().isEmpty()) {
        QPainter p(this);
        QFont f = font();
        f.setItalic(true);
        p.setFont(f);

        p.setPen(palette().color(QPalette::Disabled, QPalette::Text));

        //FIXME: fugly alert!
        // qlineedit uses an internal qstyleoption set to figure this out
        // and then adds a hardcoded 2 pixel interior to that.
        // probably requires fixes to Qt itself to do this cleanly
        // see define horizontalMargin 2 in qlineedit.cpp
        QStyleOptionFrame opt;
        initStyleOption(&opt);
        QRect cr = style()->subElementRect(QStyle::SE_LineEditContents, &opt, this);
        cr.setLeft(cr.left() + 2);
        cr.setRight(cr.right() - 2);

        p.drawText(cr, Qt::AlignLeft|Qt::AlignVCenter, d->clickMessage);
    }
}

void KLineEdit::focusInEvent( QFocusEvent *ev )
{
    if ( d->enableClickMsg && d->drawClickMsg )
    {
        d->drawClickMsg = false;
        update();
    }
    QLineEdit::focusInEvent( ev );
}

void KLineEdit::focusOutEvent( QFocusEvent *ev )
{
    if ( d->enableClickMsg && text().isEmpty() )
    {
        d->drawClickMsg = true;
        update();
    }
    QLineEdit::focusOutEvent( ev );
}

void KLineEdit::setClickMessage( const QString &msg )
{
    d->enableClickMsg = true;
    d->clickMessage = msg;
    d->drawClickMsg = text().isEmpty();
    update();
}

void KLineEdit::setContextMenuEnabled( bool showMenu )
{
    QLineEdit::setContextMenuPolicy( showMenu ? Qt::DefaultContextMenu : Qt::NoContextMenu );
}

bool KLineEdit::isContextMenuEnabled() const
{
    return  ( contextMenuPolicy() == Qt::DefaultContextMenu );
}

void KLineEdit::setPasswordMode(bool b)
{
    if(b)
    {
        KConfigGroup cg(KGlobal::config(), "Passwords");
        const QString val = cg.readEntry("EchoMode", "OneStar");
        if (val == "NoEcho")
            setEchoMode(NoEcho);
        else {
            d->threeStars = (val == "ThreeStars");
            setEchoMode(Password);
        }
    }
    else
    {
        setEchoMode( Normal );
    }
}

bool KLineEdit::passwordMode() const
{
    return echoMode() == NoEcho || echoMode() == Password;
}

void KLineEditPrivate::doCompletion(const QString& txt)
{
    if (q->emitSignals()) {
        emit q->completion(txt); // emit when requested...
    }

    if (q->handleSignals()) {
        q->makeCompletion(txt);  // handle when requested...
    }
}

#include "klineedit.moc"
#include "klineedit_p.moc"


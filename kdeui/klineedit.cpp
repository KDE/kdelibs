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
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qclipboard.h>
#include <qtimer.h>

#include <kconfig.h>
#include <kcursor.h>
#include <klocale.h>
#include <kstdaccel.h>
#include <kpopupmenu.h>
#include <kdebug.h>
#include <kcompletionbox.h>
#include <kurl.h>
#include <kurldrag.h>
#include <kiconloader.h>

#include "klineedit.h"
#include "klineedit.moc"


class KLineEdit::KLineEditPrivate
{
public:
    KLineEditPrivate()
    {
        grabReturnKeyEvents = false;
        handleURLDrops = true;
        completionBox = 0L;
        userSelection = true;
        autoSuggest = false;
        disableRestoreSelection = false;
        
        KConfigGroup config( KGlobal::config(), "General" );
        backspacePerformsCompletion = config.readBoolEntry( "Backspace performs completion", false );

    }
    ~KLineEditPrivate()
    {
        delete completionBox;
    }

    bool grabReturnKeyEvents         : 1;
    bool handleURLDrops              : 1;
    bool userSelection               : 1;
    bool autoSuggest                 : 1;
    bool disableRestoreSelection : 1;
    static bool backspacePerformsCompletion; // Configuration option
    QColor previousHighlightColor;
    QColor previousHighlightedTextColor;
    KCompletionBox *completionBox;
};

bool KLineEdit::KLineEditPrivate::backspacePerformsCompletion = false;


KLineEdit::KLineEdit( const QString &string, QWidget *parent, const char *name )
    : QLineEdit( string, parent, name )
{
    init();
}

KLineEdit::KLineEdit( QWidget *parent, const char *name )
    : QLineEdit( parent, name )
{
    init();
}

KLineEdit::~KLineEdit ()
{
    delete d;
}

void KLineEdit::init()
{
    d = new KLineEditPrivate;
    possibleTripleClick = false;
    // Enable the context menu by default.
    setContextMenuEnabled( true );
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
    QString txt = text();
    if ( t != txt )
    {
        int curpos = marked ? txt.length() : t.length();
        validateAndSet( t, curpos, curpos, t.length() );
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
       QString input = (type == KCompletionBase::PrevCompletionMatch) ? comp->previousMatch() : comp->nextMatch();
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
    if ( mode == KGlobalSettings::CompletionPopup || mode == KGlobalSettings::CompletionPopupAuto )
    {
        if ( match.isNull() )
        {
            if ( d->completionBox ) {
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

void KLineEdit::keyPressEvent( QKeyEvent *e )
{
    KKey key( e );
    if ( KStdAccel::deleteWordBack().contains( key ) )
    {
        cursorWordBackward(TRUE);
        if ( hasSelectedText() )
            del();

        e->accept();
        return;
    }
    else if ( KStdAccel::deleteWordForward().contains( key ) )
    {
        cursorWordForward(TRUE);
        if ( hasSelectedText() )
            del();

        e->accept();
        return;
    }

    // Filter key-events if EchoMode is normal &
    // completion mode is not set to CompletionNone
    if ( echoMode() == QLineEdit::Normal &&
         completionMode() != KGlobalSettings::CompletionNone )
    {
        KeyBindingMap keys = getKeyBindings();
        KGlobalSettings::Completion mode = completionMode();
        bool noModifier = (e->state() == NoButton || e->state()== ShiftButton);

        if ( ( mode == KGlobalSettings::CompletionAuto || mode == KGlobalSettings::CompletionPopupAuto ) && noModifier )
	{
            if ( !d->userSelection && ( e->key() == Key_Left || e->key() == Key_Right ) &&
                 e->state()==NoButton && d->backspacePerformsCompletion )
            {

                QString old_txt = text();
                d->disableRestoreSelection=true;

                deselect();
                QLineEdit::keyPressEvent ( e );
                int cPosition=cursorPosition();
                validateAndSet(old_txt, cPosition, cPosition, old_txt.length());
	
                d->disableRestoreSelection=false;
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
                QLineEdit::keyPressEvent ( e );

                return;
            }

	}


        if ( (mode == KGlobalSettings::CompletionAuto ||
              mode == KGlobalSettings::CompletionMan) && noModifier )
        {
            QString keycode = e->text();
            if ( !keycode.isNull() && (keycode.unicode()->isPrint() || (e->key() == Key_Backspace )))
            {
                bool hasUserSelection=d->userSelection;
                bool hadSelection=hasSelectedText();
	
                QLineEdit::keyPressEvent ( e );
                QString txt = text();
                int len = txt.length();
                if ( !hasSelectedText() && len && cursorPosition() == len )
                {
                    if ( e->key() == Key_Backspace )
                    {
                        if ( hadSelection && !hasUserSelection && d->backspacePerformsCompletion )
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

		    if(  (e->key() == Key_Backspace || e->key() == Key_Delete ) )
                        d->autoSuggest=true;

                    e->accept();
                }
                return;
            }

        }

        else if (( mode == KGlobalSettings::CompletionPopupAuto || mode == KGlobalSettings::CompletionPopup ) && noModifier )
        {
            QString old_txt = text();

	    bool hasUserSelection=d->userSelection;
	    bool hadSelection=hasSelectedText();

            uint selectedLength=selectedText().length();

            QLineEdit::keyPressEvent ( e );
		
            if (( selectedLength != selectedText().length() ) && !hasUserSelection )
                slotRestoreSelectionColors(); // and set userSelection to true
	
            QString txt = text();
            int len = txt.length();
            QString keycode = e->text();

            if ( txt != old_txt && len && cursorPosition() == len &&
                 ( (!keycode.isNull() && keycode.unicode()->isPrint()) ||
                   e->key() == Key_Backspace || e->key() == Key_Delete) )
            {

                if (e->key() == Key_Backspace && d->completionBox &&
		    d->completionBox->isVisible() ) 
                {
                    
                    if ( hadSelection && !hasUserSelection && d->backspacePerformsCompletion )
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

		if ( (e->key() == Key_Backspace || e->key() == Key_Delete ) && mode ==  KGlobalSettings::CompletionPopupAuto )
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
                cut = KStdAccel::key(KStdAccel::NextCompletion);
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

    uint selectedLength=selectedText().length();
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
        return;
    }
    QLineEdit::mousePressEvent( e );
}

void KLineEdit::tripleClickTimeout()
{
    possibleTripleClick=false;
}

QPopupMenu *KLineEdit::createPopupMenu()
{
    // Return if popup menu is not enabled !!
    if ( !m_bEnableMenu )
        return 0;

    QPopupMenu *popup = QLineEdit::createPopupMenu();

    // completion object is present.
    if ( compObj() )
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
        subMenu->insertItem( i18n("Short Automatic"), SemiAutoCompletion );
        subMenu->insertItem( i18n("Dropdown List") + QString(" + ") +
                             i18n("Automatic"), PopupAutoCompletion );

        KGlobalSettings::Completion mode = completionMode();
        subMenu->setItemChecked( NoCompletion,
                                 mode == KGlobalSettings::CompletionNone );
        subMenu->setItemChecked( ShellCompletion,
                                 mode == KGlobalSettings::CompletionShell );
        subMenu->setItemChecked( PopupCompletion,
                                 mode == KGlobalSettings::CompletionPopup );
        subMenu->setItemChecked( AutoCompletion,
                                 mode == KGlobalSettings::CompletionAuto );
        subMenu->setItemChecked( SemiAutoCompletion,
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
           setCompletionMode( KGlobalSettings::completionMode() ); break;
        case NoCompletion:
           setCompletionMode( KGlobalSettings::CompletionNone ); break;
        case AutoCompletion:
            setCompletionMode( KGlobalSettings::CompletionAuto ); break;
        case SemiAutoCompletion:
            setCompletionMode( KGlobalSettings::CompletionMan ); break;
        case ShellCompletion:
            setCompletionMode( KGlobalSettings::CompletionShell ); break;
        case PopupCompletion:
            setCompletionMode( KGlobalSettings::CompletionPopup ); break;
        case PopupAutoCompletion:
            setCompletionMode( KGlobalSettings::CompletionPopupAuto ); break;
        default: return;
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

                // Qt will emit returnPressed() itself if we return false
                if ( d->grabReturnKeyEvents || trap )
                    emit QLineEdit::returnPressed();

                emit returnPressed( displayText() );

                if ( trap )
		{
		    d->completionBox->hide();
		    deselect();
		    setCursorPosition(text().length());
		}

                // Eat the event if the user asked for it, or if a completionbox was visible
                return d->grabReturnKeyEvents || trap;
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
    QLineEdit::setText( url.prettyURL() );
}

void KLineEdit::makeCompletionBox()
{
    if ( d->completionBox )
        return;

    d->completionBox = new KCompletionBox( this, "completion box" );
    if ( handleSignals() )
    {
        connect( d->completionBox, SIGNAL(highlighted( const QString& )),
                 SLOT(setText( const QString& )) );
        connect( d->completionBox, SIGNAL(userCancelled( const QString& )),
                 SLOT(userCancelled( const QString& )) );

        // Nice lil' hacklet ;) KComboBox doesn't know when the completionbox
        // is created (childEvent() is even more hacky, IMHO), so we simply
        // forward the completionbox' activated signal from here.
        if ( parentWidget() && parentWidget()->inherits("KComboBox") )
            connect( d->completionBox, SIGNAL( activated( const QString& )),
                     parentWidget(), SIGNAL( activated( const QString & )));
    }
}

void KLineEdit::userCancelled(const QString & cancelText)
{
    if ( completionMode() != KGlobalSettings::CompletionPopupAuto ) setText(cancelText);
    else
    if (hasSelectedText() )
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

  if (KStdAccel::deleteWordBack().contains( key ))
    return true;
  if (KStdAccel::deleteWordForward().contains( key ))
    return true;

  if (d->completionBox && d->completionBox->isVisible ())
    if (e->key () == Key_Backtab)
      return true;

  return false;
}

void KLineEdit::setCompletedItems( const QStringList& items )
{
    QString txt = text();
    if ( !items.isEmpty() &&
         !(items.count() == 1 && txt == items.first()) )
    {
        if ( !d->completionBox )
            makeCompletionBox();

        if ( !txt.isEmpty() )
            d->completionBox->setCancelledText( txt );
        d->completionBox->clear();
        d->completionBox->insertItems( items );
        d->completionBox->popup();
        if ( d->autoSuggest )
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
    QPalette p=palette();
    if (userSelection)
    {
        p.setColor(QColorGroup::Highlight, d->previousHighlightColor);
        p.setColor(QColorGroup::HighlightedText, d->previousHighlightedTextColor);
    }
    else
    {
        QColor color=p.color(QPalette::Disabled, QColorGroup::Text);
        p.setColor(QColorGroup::HighlightedText, color);
        p.setColor(QColorGroup::Highlight, Qt::white);
    }
    d->userSelection=userSelection;
    setPalette(p);
}

void KLineEdit::slotRestoreSelectionColors()
{
    if (d->disableRestoreSelection) return;
    setUserSelection(true);
}

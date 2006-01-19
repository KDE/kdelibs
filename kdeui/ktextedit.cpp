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

//#include <ksyntaxhighlighter.h>
//#include <kspell.h>
#include <kcursor.h>
#include <kglobalsettings.h>
#include <kstdaccel.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdialog.h>
#include <QMenu>

class KTextEdit::KTextEditPrivate
{
public:
    KTextEditPrivate()
        : customPalette( false ),
          checkSpellingEnabled( false )
//          highlighter( 0 ),
//          spell( 0 )
    {}
    ~KTextEditPrivate() {
//        delete highlighter;
//        delete spell;
    }

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


KTextEdit::KTextEdit( const QString& text, QWidget *parent )
    : QTextEdit( text, parent ), d( new KTextEditPrivate() )
{
    KCursor::setAutoHideCursor( this, true, false );
}

KTextEdit::KTextEdit( QWidget *parent )
    : QTextEdit( parent ), d( new KTextEditPrivate() )
{
    KCursor::setAutoHideCursor( this, true, false );
}

KTextEdit::~KTextEdit()
{
    delete d;
}

void KTextEdit::keyPressEvent( QKeyEvent *e )
{
    KKey key( e );

    if ( KStdAccel::copy().contains( key ) ) {
        copy();
        e->accept();
        return;
    }
    else if ( KStdAccel::paste().contains( key ) ) {
        paste();
        e->accept();
        return;
    }
    else if ( KStdAccel::cut().contains( key ) ) {
        cut();
        e->accept();
        return;
    }
    else if ( KStdAccel::undo().contains( key ) ) {
        document()->undo();
        e->accept();
        return;
    }
    else if ( KStdAccel::redo().contains( key ) ) {
        document()->redo();
        e->accept();
        return;
    }
    else if ( KStdAccel::deleteWordBack().contains( key ) )
    {
        deleteWordBack();
        e->accept();
        return;
    }
    else if ( KStdAccel::deleteWordForward().contains( key ) )
    {
        deleteWordForward();
        e->accept();
        return;
    }
    else if ( KStdAccel::backwardWord().contains( key ) )
    {
        QTextCursor cursor = textCursor();
        cursor.movePosition( QTextCursor::PreviousWord );
        setTextCursor( cursor );
        e->accept();
        return;
    }
    else if ( KStdAccel::forwardWord().contains( key ) )
    {
        QTextCursor cursor = textCursor();
        cursor.movePosition( QTextCursor::NextWord );
        setTextCursor( cursor );
        e->accept();
        return;
    }
    else if ( KStdAccel::next().contains( key ) )
    {
        QTextCursor cursor = textCursor();
        int targetY = verticalScrollBar()->value() + viewport()->height();
        bool moved = false;
        do
        {
            moved = cursor.movePosition( QTextCursor::Down );
            setTextCursor( cursor );
        }
        while ( moved && verticalScrollBar()->value() < targetY );
        e->accept();
        return;
    }
    else if ( KStdAccel::prior().contains( key ) )
    {
        QTextCursor cursor = textCursor();
        int targetY = verticalScrollBar()->value() - viewport()->height();
        bool moved = false;
        do
        {
            moved = cursor.movePosition( QTextCursor::Up );
            setTextCursor( cursor );
        }
        while ( moved && verticalScrollBar()->value() > targetY );
        e->accept();
        return;
    }
    else if ( KStdAccel::home().contains( key ) )
    {
        QTextCursor cursor = textCursor();
        cursor.movePosition( QTextCursor::Start );
        setTextCursor( cursor );
        e->accept();
        return;
    }
    else if ( KStdAccel::end().contains( key ) )
    {
        QTextCursor cursor = textCursor();
        cursor.movePosition( QTextCursor::End );
        setTextCursor( cursor );
        e->accept();
        return;
    }
    else if ( KStdAccel::beginningOfLine().contains( key ) )
    {
        QTextCursor cursor = textCursor();
        cursor.movePosition( QTextCursor::StartOfLine );
        setTextCursor( cursor );
        e->accept();
        return;
    }
    else if ( KStdAccel::endOfLine().contains( key ) )
    {
        QTextCursor cursor = textCursor();
        cursor.movePosition( QTextCursor::EndOfLine );
        setTextCursor( cursor );
        e->accept();
        return;
    }
    else if ( KStdAccel::pasteSelection().contains( key ) )
    {
        QString text = QApplication::clipboard()->text( QClipboard::Selection );
        if ( !text.isEmpty() )
            insertPlainText( text );  // TODO: check if this is html? (MiB)
        e->accept();
        return;
    }

    // ignore Ctrl-Return so that KDialogs can close the dialog
    else if ( e->state() == Qt::ControlModifier &&
              (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) &&
              qobject_cast<KDialog*>(topLevelWidget()) )
    {
        e->ignore();
        return;
    }

    QTextEdit::keyPressEvent( e );
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

void KTextEdit::slotAllowTab()
{
    setTabChangesFocus( !tabChangesFocus() );
}

void KTextEdit::menuActivated( QAction* act)
{
    if ( act == d->spellCheckAction )
        checkSpelling();
    else if (  act == d->autoSpellCheckAction )
        toggleAutoSpellCheck();
    else if ( act == d->allowTab )
        slotAllowTab();
}

void KTextEdit::contextMenuEvent( QContextMenuEvent *e )
{
    QMenu *popup = createStandardContextMenu();
    connect( popup, SIGNAL( triggered ( QAction* ) ),
             this, SLOT( menuActivated( QAction* ) ) );

    popup->addSeparator();
    d->spellCheckAction = popup->addAction( SmallIconSet( "spellcheck" ), i18n( "Check Spelling..." ) );

    if ( text().isEmpty() )
        d->spellCheckAction->setEnabled( false );

    d->autoSpellCheckAction = popup->addAction( i18n( "Auto Spell Check" ) );
    d->autoSpellCheckAction->setCheckable(true);
    d->autoSpellCheckAction->setChecked(d->checkSpellingEnabled);
    popup->addSeparator();
    d->allowTab = popup->addAction(i18n("Allow Tabulations"));
    d->allowTab->setCheckable(true);
    d->allowTab->setChecked(!tabChangesFocus());
    popup->exec(e->globalPos());
    delete popup;


// TODO
/*
    enum { IdUndo, IdRedo, IdSep1, IdCut, IdCopy, IdPaste, IdClear, IdSep2, IdSelectAll };

    QMenu *menu = createStandardContextMenu();

    if ( isReadOnly() )
        menu->changeItem( menu->idAt(0), SmallIconSet("editcopy"), menu->text( menu->idAt(0) ) );
    else {
        int id = menu->idAt(0);
        menu->changeItem( id - IdUndo, SmallIconSet("undo"), menu->text( id - IdUndo) );
        menu->changeItem( id - IdRedo, SmallIconSet("redo"), menu->text( id - IdRedo) );
        menu->changeItem( id - IdCut, SmallIconSet("editcut"), menu->text( id - IdCut) );
        menu->changeItem( id - IdCopy, SmallIconSet("editcopy"), menu->text( id - IdCopy) );
        menu->changeItem( id - IdPaste, SmallIconSet("editpaste"), menu->text( id - IdPaste) );
        menu->changeItem( id - IdClear, SmallIconSet("editclear"), menu->text( id - IdClear) );

        menu->insertSeparator();
        id = menu->insertItem( SmallIconSet( "spellcheck" ), i18n( "Check Spelling..." ),
                               this, SLOT( checkSpelling() ) );

        if ( text().isEmpty() )
            menu->setItemEnabled( id, false );

        id = menu->insertItem( i18n( "Auto Spell Check" ),
                               this, SLOT( toggleAutoSpellCheck() ) );
        menu->setItemChecked(id, d->checkSpellingEnabled);
        menu->insertSeparator();
        id = menu->insertItem(i18n("Allow Tabulations"),this,SLOT(slotAllowTab()));
        menu->setItemChecked(id, !tabChangesFocus());
    }
*/

}

void KTextEdit::wheelEvent( QWheelEvent *e )
{
    if ( KGlobalSettings::wheelMouseZooms() )
        QTextEdit::wheelEvent( e );
    else // thanks, we don't want to zoom, so skip QTextEdit's impl.
        QAbstractScrollArea::wheelEvent( e );
}

void KTextEdit::toggleAutoSpellCheck()
{
//    setCheckSpellingEnabled( !d->checkSpellingEnabled );
}

void KTextEdit::setCheckSpellingEnabled( bool check )
{
#if 0
    if ( check == d->checkSpellingEnabled )
        return;

    // From the above statment we know know that if we're turning checking
    // on that we need to create a new highlighter and if we're turning it
    // off we should remove the old one.

    d->checkSpellingEnabled = check;
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

void KTextEdit::focusInEvent( QFocusEvent *e )
{
#if 0
    if ( d->checkSpellingEnabled && !d->highlighter )
        d->highlighter = new KDictSpellingHighlighter( this );
#endif

    QTextEdit::focusInEvent( e );
}

bool KTextEdit::checkSpellingEnabled() const
{
//    return d->checkSpellingEnabled;
    return false;
}

void KTextEdit::setReadOnly( bool readOnly )
{
    if ( readOnly == isReadOnly() )
        return;

    if ( readOnly )
    {
        d->customPalette = testAttribute( Qt::WA_SetPalette );
        QPalette p = palette();
        QColor color = p.color( QPalette::Disabled, QColorGroup::Background );
        p.setColor( QColorGroup::Base, color );
        p.setColor( QColorGroup::Background, color );
        setPalette( p );
    }
    else
    {
        if ( d->customPalette && testAttribute( Qt::WA_SetPalette ) )
        {
            QPalette p = palette();
            QColor color = p.color( QPalette::Normal, QColorGroup::Base );
            p.setColor( QColorGroup::Base, color );
            p.setColor( QColorGroup::Background, color );
            setPalette( p );
        }
        else
            setPalette( QPalette() );
    }

    QTextEdit::setReadOnly( readOnly );
}

void KTextEdit::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

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

void KTextEdit::spellCheckerMisspelling( const QString &text, const QStringList &, unsigned int pos )
{
//    highLightWord( text.length(), pos );
}

void KTextEdit::spellCheckerCorrected( const QString &oldWord, const QString &newWord, unsigned int pos )
{
   // TODO
/*    unsigned int l = 0;
    unsigned int cnt = 0;
    if ( oldWord != newWord ) {
        posToRowCol( pos, l, cnt );
        setSelection( l, cnt, l, cnt + oldWord.length() );
        removeSelectedText();
        insert( newWord );
    }*/
}

void KTextEdit::posToRowCol( unsigned int pos, unsigned int &line, unsigned int &col )
{
// TODO
/*
    for ( line = 0; line < static_cast<uint>( lines() ) && col <= pos; line++ )
        col += paragraphLength( line ) + 1;

    line--;
    col = pos - col + paragraphLength( line ) + 1;
*/
}

void KTextEdit::spellCheckerFinished()
{
//    delete d->spell;
//    d->spell = 0L;
}

void KTextEdit::slotSpellCheckReady( KSpell *s )
{
// this is for Zack...
//
//    s->check( text() ); // TODO: toPlainText()?? (MiB)
//    connect( s, SIGNAL( done( const QString & ) ), this, SLOT( slotSpellCheckDone( const QString & ) ) );
}

void KTextEdit::slotSpellCheckDone( const QString &s )
{
//    if ( s != text() ) // TODO: toPlainText()?? (MiB)
//        setText( s ); // setPlainText() ?! we'd loose rich text info
}


void KTextEdit::highLightWord( unsigned int length, unsigned int pos )
{
   /* TODO
    unsigned int l = 0;
    unsigned int cnt = 0;
    posToRowCol( pos, l, cnt );
    setSelection( l, cnt, l, cnt + length );
    */
}

#include "ktextedit.moc"

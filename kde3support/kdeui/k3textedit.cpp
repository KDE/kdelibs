/* This file is part of the KDE libraries
   Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

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

#include "k3textedit.h"

#include <qapplication.h>
#include <qclipboard.h>
#include <q3popupmenu.h>

#include <k3syntaxhighlighter.h>
#include <kspell.h>
#include <kcursor.h>
#include <kglobalsettings.h>
#include <kstdaccel.h>
#include <kiconloader.h>
#include <klocale.h>
#include <QKeyEvent>

class K3TextEdit::K3TextEditPrivate
{
public:
    K3TextEditPrivate()
        : customPalette( false ),
          checkSpellingEnabled( false ),
          highlighter( 0 ),
          spell( 0 )
    {}
    ~K3TextEditPrivate() {
        delete highlighter;
        delete spell;
    }

    bool customPalette;
    bool checkSpellingEnabled;
    K3DictSpellingHighlighter *highlighter;
    KSpell *spell;
};

K3TextEdit::K3TextEdit( const QString& text, const QString& context,
                      QWidget *parent, const char *name )
    : Q3TextEdit ( text, context, parent, name )
{
    d = new K3TextEditPrivate();
    KCursor::setAutoHideCursor( this, true, false );
}

K3TextEdit::K3TextEdit( QWidget *parent, const char *name )
    : Q3TextEdit ( parent, name )
{
    d = new K3TextEditPrivate();
    KCursor::setAutoHideCursor( this, true, false );
}

K3TextEdit::~K3TextEdit()
{
    delete d;
}

void K3TextEdit::keyPressEvent( QKeyEvent *e )
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
        undo();
        e->accept();
        return;
    }
    else if ( KStdAccel::redo().contains( key ) ) {
        redo();
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
      CursorAction action = MoveWordBackward;
      int para, index;
      getCursorPosition( &para, & index );
      if (text(para).isRightToLeft())
           action = MoveWordForward;
      moveCursor(action, false );
      e->accept();
      return;
    }
    else if ( KStdAccel::forwardWord().contains( key ) )
    {
      CursorAction action = MoveWordForward;
      int para, index;
      getCursorPosition( &para, & index );
      if (text(para).isRightToLeft())
	  action = MoveWordBackward;
      moveCursor( action, false );
      e->accept();
      return;
    }
    else if ( KStdAccel::next().contains( key ) )
    {
      moveCursor( MovePgDown, false );
      e->accept();
      return;
    }
    else if ( KStdAccel::prior().contains( key ) )
    {
      moveCursor( MovePgUp, false );
      e->accept();
      return;
    }
    else if ( KStdAccel::home().contains( key ) )
    {
      moveCursor( MoveHome, false );
      e->accept();
      return;
    }
    else if ( KStdAccel::end().contains( key ) )
    {
      moveCursor( MoveEnd, false );
      e->accept();
      return;
    }
    else if ( KStdAccel::beginningOfLine().contains( key ) )
    {
      moveCursor( MoveLineStart, false );
      e->accept();
      return;
    }
    else if ( KStdAccel::endOfLine().contains( key ) )
    {
      moveCursor(MoveLineEnd, false);
      e->accept();
      return;
    }
    else if ( KStdAccel::pasteSelection().contains( key ) )
    {
        QString text = QApplication::clipboard()->text( QClipboard::Selection);
        if ( !text.isEmpty() )
            insert( text );
        e->accept();
        return;
    }

    // ignore Ctrl-Return so that KDialogs can close the dialog
    else if ( e->state() == Qt::ControlModifier &&
              (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) &&
              topLevelWidget()->inherits( "KDialog" ) )
    {
        e->ignore();
        return;
    }
    
    Q3TextEdit::keyPressEvent( e );
}

void K3TextEdit::deleteWordBack()
{
    removeSelection();
    moveCursor( MoveWordBackward, true );
    removeSelectedText();
}

void K3TextEdit::deleteWordForward()
{
    removeSelection();
    moveCursor( MoveWordForward, true );
    removeSelectedText();
}

void K3TextEdit::slotAllowTab()
{
setTabChangesFocus(!tabChangesFocus());
}

Q3PopupMenu *K3TextEdit::createPopupMenu( const QPoint &pos )
{
    enum { IdUndo, IdRedo, IdSep1, IdCut, IdCopy, IdPaste, IdClear, IdSep2, IdSelectAll };

    Q3PopupMenu *menu = Q3TextEdit::createPopupMenu( pos );

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

        if( text().isEmpty() )
            menu->setItemEnabled( id, false );

        id = menu->insertItem( i18n( "Auto Spell Check" ),
                               this, SLOT( toggleAutoSpellCheck() ) );
        menu->setItemChecked(id, d->checkSpellingEnabled);
	menu->insertSeparator();
	id=menu->insertItem(i18n("Allow Tabulations"),this,SLOT(slotAllowTab()));
	menu->setItemChecked(id, !tabChangesFocus());
    }

    return menu;
}

Q3PopupMenu *K3TextEdit::createPopupMenu()
{
    return Q3TextEdit::createPopupMenu();
}

void K3TextEdit::contentsWheelEvent( QWheelEvent *e )
{
    if ( KGlobalSettings::wheelMouseZooms() )
        Q3TextEdit::contentsWheelEvent( e );
    else // thanks, we don't want to zoom, so skip QTextEdit's impl.
        Q3ScrollView::contentsWheelEvent( e );
}

void K3TextEdit::setPalette( const QPalette& palette )
{
    Q3TextEdit::setPalette( palette );
    // unsetPalette() is not virtual and calls setPalette() as well
    // so we can use ownPalette() to find out about unsetting
    d->customPalette = ownPalette();
}

void K3TextEdit::toggleAutoSpellCheck()
{
    setCheckSpellingEnabled( !d->checkSpellingEnabled );
}

void K3TextEdit::setCheckSpellingEnabled( bool check )
{
    if ( check == d->checkSpellingEnabled )
        return;

    // From the above statment we know know that if we're turning checking
    // on that we need to create a new highlighter and if we're turning it
    // off we should remove the old one.

    d->checkSpellingEnabled = check;
    if ( check )
    {
        if (hasFocus())
            d->highlighter = new K3DictSpellingHighlighter( this );
    }
    else
    {
        delete d->highlighter;
        d->highlighter = 0;
    }
}

void K3TextEdit::focusInEvent( QFocusEvent *e )
{
    if ( d->checkSpellingEnabled && !d->highlighter )
        d->highlighter = new K3DictSpellingHighlighter( this );

    Q3TextEdit::focusInEvent( e );
}

bool K3TextEdit::checkSpellingEnabled() const
{
    return d->checkSpellingEnabled;
}

void K3TextEdit::setReadOnly(bool readOnly)
{
    if ( readOnly == isReadOnly() )
        return;

    if (readOnly)
    {
        bool custom = ownPalette();
        QPalette p = palette();
        QColor color = p.color(QPalette::Disabled, QColorGroup::Background);
        p.setColor(QColorGroup::Base, color);
        p.setColor(QColorGroup::Background, color);
        setPalette(p);
        d->customPalette = custom;
    }
    else
    {
        if ( d->customPalette )
        {
            QPalette p = palette();
            QColor color = p.color(QPalette::Normal, QColorGroup::Base);
            p.setColor(QColorGroup::Base, color);
            p.setColor(QColorGroup::Background, color);
            setPalette( p );
        }
        else
            unsetPalette();
    }

    Q3TextEdit::setReadOnly (readOnly);
}

void K3TextEdit::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void K3TextEdit::checkSpelling()
{
    delete d->spell;
    d->spell = new KSpell( this, i18n( "Spell Checking" ),
                          this, SLOT( slotSpellCheckReady( KSpell *) ), 0, true, true);

    connect( d->spell, SIGNAL( death() ),
             this, SLOT( spellCheckerFinished() ) );

    connect( d->spell, SIGNAL( misspelling( const QString &, const QStringList &, unsigned int ) ),
             this, SLOT( spellCheckerMisspelling( const QString &, const QStringList &, unsigned int ) ) );

    connect( d->spell, SIGNAL( corrected( const QString &, const QString &, unsigned int ) ),
             this, SLOT( spellCheckerCorrected( const QString &, const QString &, unsigned int ) ) );
}

void K3TextEdit::spellCheckerMisspelling( const QString &text, const QStringList &, unsigned int pos )
{
    highLightWord( text.length(), pos );
}

void K3TextEdit::spellCheckerCorrected( const QString &oldWord, const QString &newWord, unsigned int pos )
{
    unsigned int l = 0;
    unsigned int cnt = 0;
    if ( oldWord != newWord ) {
        posToRowCol( pos, l, cnt );
        setSelection( l, cnt, l, cnt + oldWord.length() );
        removeSelectedText();
        insert( newWord );
    }
}

void K3TextEdit::posToRowCol(unsigned int pos, unsigned int &line, unsigned int &col)
{
    for ( line = 0; line < static_cast<uint>( lines() ) && col <= pos; line++ )
        col += paragraphLength( line ) + 1;

    line--;
    col = pos - col + paragraphLength( line ) + 1;
}

void K3TextEdit::spellCheckerFinished()
{
    delete d->spell;
    d->spell = 0L;
}

void K3TextEdit::slotSpellCheckReady( KSpell *s )
{
    s->check( text() );
    connect( s, SIGNAL( done( const QString & ) ), this, SLOT( slotSpellCheckDone( const QString & ) ) );
}

void K3TextEdit::slotSpellCheckDone( const QString &s )
{
    if ( s != text() )
        setText( s );
}


void K3TextEdit::highLightWord( unsigned int length, unsigned int pos )
{
    unsigned int l = 0;
    unsigned int cnt = 0;
    posToRowCol( pos, l, cnt );
    setSelection( l, cnt, l, cnt + length );
}

#include "k3textedit.moc"

/* This file is part of the KDE libraries

   Copyright (C) 1997 Sven Radej (sven.radej@iname.com)
   Copyright (c) 1999 Patrick Ward <PAT_WARD@HP-USA-om5.om.hp.com>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qlineedit.h>
#include <qpopupmenu.h>
#include <qkeycode.h>

#include "klined.h"
#include "klined.moc"

#ifdef KeyPress
#undef KeyPress
#endif

KLineEdit::KLineEdit(const QString &string, QWidget *parent, const char *name)
    : QLineEdit(string, parent, name)
{
    pop = new QPopupMenu( this );
    pmid[0] = pop->insertItem( "Cut", this, SLOT(doCut()) );
    pmid[1] = pop->insertItem( "Copy", this, SLOT(doCopy()) );
    pmid[2] = pop->insertItem( "Paste", this, SLOT(doPaste()) );
    pmid[3] = pop->insertItem( "Clear", this, SLOT(doClear()) );
    pop->insertSeparator();
    pmid[4] = pop->insertItem( "Select All", this, SLOT(doSelect()) );

    installEventFilter (this);
}

KLineEdit::KLineEdit (QWidget *parent, const char *name)
    : QLineEdit (parent, name)
{
    pop = new QPopupMenu( this );
    pmid[0] = pop->insertItem( "Cut", this, SLOT(doCut()) );
    pmid[1] = pop->insertItem( "Copy", this, SLOT(doCopy()) );
    pmid[2] = pop->insertItem( "Paste", this, SLOT(doPaste()) );
    pmid[3] = pop->insertItem( "Clear", this, SLOT(doClear()) );
    pop->insertSeparator();
    pmid[4] = pop->insertItem( "Select All", this, SLOT(doSelect()) );

    installEventFilter (this);
}

KLineEdit::~KLineEdit ()
{
    removeEventFilter (this);
    delete pop;
    pop = 0;
}

void KLineEdit::cursorAtEnd ()
{
    QKeyEvent ev( QEvent::KeyPress, Key_End, 0, 0 );
    QLineEdit::keyPressEvent( &ev );
}

bool KLineEdit::eventFilter (QObject *, QEvent *e)
{
    if (e->type() == QEvent::KeyPress)
    {
	QKeyEvent *k = (QKeyEvent *) e;

	if ( ((k->state() == ControlButton) && (k->key() == Key_S || k->key() == Key_D))
	     || k->key() == Key_Tab) {
	    if (k->key() == Key_Tab || k->key() == Key_S)
		emit completion ();
	    else
		emit rotation ();
	    cursorAtEnd();
	    return TRUE;
	}
    }
    return FALSE;
}

void KLineEdit::mousePressEvent( QMouseEvent *e )
{
    if(e->button() != RightButton) {
	QLineEdit::mousePressEvent( e );
	return;
    }

    int n = text().length();
    pop->setItemEnabled( pmid[0], hasMarkedText() );
    pop->setItemEnabled( pmid[1], hasMarkedText() );
    pop->setItemEnabled( pmid[3], (n > 0) ? true : false );
    pop->popup( e->globalPos() );
}


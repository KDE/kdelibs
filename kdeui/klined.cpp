/* This file is part of the KDE libraries
    Copyright (C) 1997 Sven Radej (sven.radej@iname.com)

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
// This was taken from filentry.cpp
// fileentry.cpp is part of KFM II, by Torben Weis
// changed to accept Tab, maybe this should be switchable (constructor)
// 'cause a tabbed dialog will stop working ..

#include <qlineedit.h>
#include "klined.h"

#include <qkeycode.h>


KLined::KLined (QWidget *parent, const char *name)
  : QLineEdit (parent, name)
{
  installEventFilter (this);
}

KLined::~KLined ()
{
  removeEventFilter (this);
}

void KLined::cursorAtEnd ()
{
  QKeyEvent ev( Event_KeyPress, Key_End, 0, 0 );
  QLineEdit::keyPressEvent( &ev );
}

bool KLined::eventFilter (QObject *, QEvent *e)
{
  if (e->type() == Event_KeyPress)
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

/**********************************************************************
**
** $Id$
**
** Implementation of KRestrictedLine
**
** Copyright (C) 1997 Michael Wiedmann, <mw@miwie.in-berlin.de>
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the Free
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
*****************************************************************************/

#include "krestrictedline.h"
#include "qkeycode.h"

#include "krestrictedline.h"


KRestrictedLine::KRestrictedLine( QWidget *parent, 
								  const char *name,
								  const char *valid )
  : QLineEdit( parent, name )
{
  qsValidChars = valid;
}

KRestrictedLine::~KRestrictedLine()
{
  ;
}


void KRestrictedLine::keyPressEvent( QKeyEvent *e )
{
  if (e->key() == Key_Enter || e->key() == Key_Return)
    {
      emit returnPressed();
      return;
    }

  // let QLineEdit process "special" keys
  // so that we still can use the default key binding
  if (e->ascii() < 32)
    {
      QLineEdit::keyPressEvent(e);
      return;
    }

  // do we have a list of valid chars &&
  // is the pressed key in the list of valid chars?
  if (!qsValidChars.isEmpty() && !qsValidChars.contains((char)e->ascii()))
    {
      // invalid char, emit signal and return
      emit (invalidChar(e->key()));
      return;
    }
  else
	// valid char: let QLineEdit process this key as usual
	QLineEdit::keyPressEvent(e);

  return;
}


void KRestrictedLine::setValidChars( const char *valid)
{
  qsValidChars = valid;
}


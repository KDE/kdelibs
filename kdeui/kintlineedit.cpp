/**********************************************************************
**
** $Id$
**
** Implementation of 
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

#include <qkeycode.h>

#include <stdlib.h>

#include "kintlineedit.h"


KIntLineEdit::KIntLineEdit( QWidget *parent, 
		const char *name, KEditLineType t)
  : KRestrictedLine( parent, name, "0123456789" )
{
  switch (lineType = t)
    {
    case KEditTypeOct:
      setValidChars("01234567");
      break;
      
    case KEditTypeHex:
      setValidChars("0123456789ABCDEFabcdef");
      break;

    default:
      break;
    }

  connect( this, SIGNAL( textChanged( const QString & ) ),
		   this, SLOT( internalValueChanged() ) );
}

KIntLineEdit::KIntLineEdit()
  : KRestrictedLine()
{
}

KIntLineEdit::~KIntLineEdit()
{
}

KEditLineType KIntLineEdit::getType()
{
  return (lineType);
}


int KIntLineEdit::value( void )
{
  QString s = text();
  return getValue( s );
}


void KIntLineEdit::setValue( int value )
{
  QString s;
  putValue( s, value );
}

 
void KIntLineEdit::keyPressEvent( QKeyEvent *e )
{
  QString tmp(text());
  int i;
  
  switch(e->key())
    {
    case Key_Up:
      putValue(tmp, getValue(tmp)+1);
      break;
      
    case Key_Down:
      i = getValue(tmp);
      if (i>0) i--;
      putValue(tmp, i);
      break;
      
    case Key_Prior:
      incValue(tmp, getValue(tmp));
      break;
            
    case Key_Next:
      decValue(tmp, getValue(tmp));
      break;
      
    default: // switch (e->key())
      KRestrictedLine::keyPressEvent(e);
      break;
    }
  
  return;
}


int KIntLineEdit::getValue(QString &s)
{
  return s.toInt(0, lineType);
}

void KIntLineEdit::putValue(QString &s, int val)
{
  switch (lineType)
    {
	case  KEditTypeOct:
	  s.setNum(val, 8);
	  break;
       
	case  KEditTypeDec:
	  s.setNum(val, 10);
	  break;

	case  KEditTypeHex:
	  s.setNum(val, 16);
	  break;
    }
  setText(s);
}

void KIntLineEdit::incValue(QString &s, int val)
{
  switch (lineType)
    {
	case  KEditTypeOct:
	  val += 8;
	  break;
       
	case  KEditTypeDec:
	  val += 10;
	  break;

	case  KEditTypeHex:
	  val +=16;
	  break;
    }
  putValue(s, val);
}

void KIntLineEdit::decValue(QString &s, int val)
{
  switch (lineType)
    {
	case  KEditTypeOct:
	  if (val > 7)   val -= 8;
	  break;
       
	case  KEditTypeDec:
	  if (val > 9)   val -= 10;
	  break;

	case  KEditTypeHex:
	  if (val > 15)   val -= 16;
	  break;
    }
  putValue(s, val);
}


void KIntLineEdit::internalValueChanged()
{
  emit valueChanged( value() );
}

#include "kintlineedit.moc"


/*  This file is part of the KDE Libraries
    Copyright (C) 1998 Thomas Tanghus (tanghus@earthling.net)
    Additions by Espen Sand (espensa@online.no) 

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

#include <kapp.h>
#include <kdialog.h>

int KDialog::mMarginSize = 6;
int KDialog::mSpacingSize = 6;



KDialog::KDialog(QWidget *parent, const char *name, bool modal, WFlags f)
  : QDialog(parent, name, modal, f)
{
  //debug("KDialog creation");
  //initMetaObject();
  setFocusPolicy(QWidget::StrongFocus);
}

// Grab QDialogs keypresses if non-modal.
void KDialog::keyPressEvent(QKeyEvent *e)
{
  if ( e->state() == 0 )
  {
    switch ( e->key() )
    {
      case Key_Escape:
      case Key_Enter:
      case Key_Return:
      {
        if(testWFlags(WType_Modal))
          QDialog::keyPressEvent(e);
        else
        {
          //debug("KDialog - Eating keyevent");
  	  e->ignore();
        }
      }
      break;
      default:
	e->ignore();
	return;
    }
  }
  else
  {
    e->ignore();
  }
}


int KDialog::marginHint( void ) const
{
  return( mMarginSize );
}


int KDialog::spacingHint( void ) const
{
  return( mSpacingSize );
}


void KDialog::setCaption( const QString &caption )
{
  QDialog::setCaption( kapp->makeStdCaption( caption ) );
}


void KDialog::setPlainCaption( const QString &caption )
{
  QDialog::setCaption( caption );
}






#include "kdialog.moc"

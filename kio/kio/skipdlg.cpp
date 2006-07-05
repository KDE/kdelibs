/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kio/skipdlg.h"

#include <stdio.h>
#include <assert.h>

#include <qmessagebox.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qlabel.h>

#include <kguiitem.h>
#include <klocale.h>
#include <kurl.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>

using namespace KIO;

SkipDlg::SkipDlg(QWidget *parent, bool _multi, const QString& _error_text, bool _modal )
  : KDialog ( parent ), modal( _modal )
{
  setCaption( i18n( "Information" ) );
  setModal( _modal );

  if ( !_multi ) {
    setButtons( Cancel );
  } else {
    setButtons( Cancel | User1 | User2 );

    setButtonText( User1, i18n( "Skip" ) );
    connect( this, SIGNAL( user1Clicked() ), SLOT( b1Pressed() ) );

    setButtonText( User2, i18n( "AutoSkip" ) );
    connect( this, SIGNAL( user2Clicked() ), SLOT( b2Pressed() ) );
  }

  connect( this, SIGNAL( cancelClicked() ), SLOT( b0Pressed() ) );

  setMainWidget( new QLabel( _error_text, this ) );

  resize( sizeHint() );
}

SkipDlg::~SkipDlg()
{
}

void SkipDlg::b0Pressed()
{
  if ( modal )
    done( 0 );
  else
    emit result( this, 0 );
}

void SkipDlg::b1Pressed()
{
  if ( modal )
    done( 1 );
  else
    emit result( this, 1 );
}

void SkipDlg::b2Pressed()
{
  if ( modal )
    done( 2 );
  else
    emit result( this, 2 );
}

SkipDlg_Result KIO::open_SkipDlg( bool _multi, const QString& _error_text )
{
  SkipDlg dlg( 0L, _multi, _error_text, true );
  return (SkipDlg_Result) dlg.exec();
}

#include "skipdlg.moc"

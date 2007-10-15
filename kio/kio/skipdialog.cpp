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

#include "kio/skipdialog.h"

#include <stdio.h>
#include <assert.h>

#include <QtGui/QMessageBox>
#include <QtGui/QWidget>
#include <QtGui/QLayout>
#include <QtGui/QLabel>

#include <kguiitem.h>
#include <klocale.h>
#include <kurl.h>
#include <kpushbutton.h>
#include <kstandardguiitem.h>

using namespace KIO;

SkipDialog::SkipDialog(QWidget *parent, bool _multi, const QString& _error_text )
  : KDialog ( parent ), d( 0 )
{
  setCaption( i18n( "Information" ) );

  if ( !_multi ) {
    setButtons( Cancel );
  } else {
    setButtons( Cancel | User1 | User2 );

    setButtonText( User1, i18n( "Skip" ) );
    connect( this, SIGNAL( user1Clicked() ), SLOT(skipPressed()) );

    setButtonText( User2, i18n( "AutoSkip" ) );
    connect( this, SIGNAL( user2Clicked() ), SLOT(autoSkipPressed()) );
  }

  connect( this, SIGNAL( cancelClicked() ), SLOT(cancelPressed()) );

  setMainWidget( new QLabel( _error_text, this ) );

  resize( sizeHint() );
}

SkipDialog::~SkipDialog()
{
}

void SkipDialog::cancelPressed()
{
  done(S_CANCEL);
}

void SkipDialog::skipPressed()
{
  done(S_SKIP);
}

void SkipDialog::autoSkipPressed()
{
  done(S_AUTO_SKIP);
}

#include "skipdialog.moc"

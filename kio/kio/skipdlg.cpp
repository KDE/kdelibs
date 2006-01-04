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

#include <kapplication.h>
#include <klocale.h>
#include <kurl.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>

#ifdef Q_WS_X11 
#include <kwin.h>
#endif

using namespace KIO;

SkipDlg::SkipDlg(QWidget *parent, bool _multi, const QString& _error_text, bool _modal ) :
  KDialog ( parent)
{
  setModal(_modal);
  // TODO : port to KDialogBase
  modal = _modal;

  // Set "StaysOnTop", because this dialog is typically used in kio_uiserver,
  // i.e. in a separate process.
#ifdef Q_WS_X11 //FIXME(E): Implement for QT Embedded, mac & win32
  if (modal)
    KWin::setState( winId(), NET::StaysOnTop );
#endif

  b0 = b1 = b2 = 0L;

  setCaption( i18n( "Information" ) );

  b0 = new KPushButton( KStdGuiItem::cancel(), this );
  connect(b0, SIGNAL(clicked()), this, SLOT(b0Pressed()));

  if ( _multi )
  {
    b1 = new QPushButton( i18n( "Skip" ), this );
    connect(b1, SIGNAL(clicked()), this, SLOT(b1Pressed()));

    b2 = new QPushButton( i18n( "Auto Skip" ), this );
    connect(b2, SIGNAL(clicked()), this, SLOT(b2Pressed()));
  }

  QVBoxLayout *vlayout = new QVBoxLayout( this, 10, 0 );
  // vlayout->addStrut( 360 );	makes dlg at least that wide

  QLabel * lb = new QLabel( _error_text, this );
  lb->setFixedHeight( lb->sizeHint().height() );
  lb->setMinimumWidth( lb->sizeHint().width() );
  vlayout->addWidget( lb );

  vlayout->addSpacing( 10 );

  QHBoxLayout* layout = new QHBoxLayout();
  vlayout->addLayout( layout );
  if ( b0 )
  {
    b0->setDefault( true );
    b0->setFixedSize( b0->sizeHint() );
    layout->addWidget( b0 );
    layout->addSpacing( 5 );
  }
  if ( b1 )
  {
    b1->setFixedSize( b1->sizeHint() );
    layout->addWidget( b1 );
    layout->addSpacing( 5 );
  }
  if ( b2 )
  {
    b2->setFixedSize( b2->sizeHint() );
    layout->addWidget( b2 );
    layout->addSpacing( 5 );
  }

  vlayout->addStretch( 10 );
  vlayout->activate();
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
  Q_ASSERT(kapp);

  SkipDlg dlg( 0L, _multi, _error_text, true );
  return (SkipDlg_Result) dlg.exec();
}

#include "skipdlg.moc"

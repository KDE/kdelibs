/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       David Faure <faure@kde.org>

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

#include "kio/renamedlg.h"

#include <stdio.h>
#include <assert.h>

#include <kmessagebox.h>
#include <qfileinfo.h>

#include <kapplication.h>
#include <kio/global.h>
#include <kdialog.h>
#include <klocale.h>
#include <kglobal.h>
#include <kurl.h>
#include <kmimetype.h>
#include <kwin.h>
#include <kstringhandler.h>

using namespace KIO;

RenameDlg::RenameDlg(QWidget *parent, const QString & _caption,
                     const QString &_src, const QString &_dest,
                     RenameDlg_Mode _mode,
                     unsigned long sizeSrc,
                     unsigned long sizeDest,
                     time_t ctimeSrc,
                     time_t ctimeDest,
                     time_t mtimeSrc,
                     time_t mtimeDest,
                     bool _modal)
  : QDialog ( parent, "KIO::RenameDialog" , _modal )
{
  modal = _modal;
  // Set "StaysOnTop", because this dialog is typically used in kio_uiserver,
  // i.e. in a separate process.
#ifndef Q_WS_QWS //FIXME(E): Implement for QT Embedded
  if (modal)
    KWin::setState( winId(), NET::StaysOnTop );
#endif

  src = _src;
  dest = _dest;

  b0 = b1 = b2 = b3 = b4 = b5 = b6 = b7 = 0L;

  setCaption( _caption );

  b0 = new QPushButton( i18n( "Cancel" ), this );
  connect(b0, SIGNAL(clicked()), this, SLOT(b0Pressed()));

  if ( ! (_mode & M_NORENAME ) ) {
      b1 = new QPushButton( i18n( "Rename" ), this );
      b1->setEnabled(false);
      connect(b1, SIGNAL(clicked()), this, SLOT(b1Pressed()));
  }

  if ( ( _mode & M_MULTI ) && ( _mode & M_SKIP ) ) {
    b2 = new QPushButton( i18n( "Skip" ), this );
    connect(b2, SIGNAL(clicked()), this, SLOT(b2Pressed()));

    b3 = new QPushButton( i18n( "Auto Skip" ), this );
    connect(b3, SIGNAL(clicked()), this, SLOT(b3Pressed()));
  }

  if ( _mode & M_OVERWRITE ) {
    b4 = new QPushButton( i18n( "Overwrite" ), this );
    connect(b4, SIGNAL(clicked()), this, SLOT(b4Pressed()));

    if ( _mode & M_MULTI ) {
      b5 = new QPushButton( i18n( "Overwrite All" ), this );
      connect(b5, SIGNAL(clicked()), this, SLOT(b5Pressed()));
    }
  }

  if ( _mode & M_RESUME ) {
    b6 = new QPushButton( i18n( "Resume" ), this );
    connect(b6, SIGNAL(clicked()), this, SLOT(b6Pressed()));

    if ( _mode & M_MULTI )
      {
	b7 = new QPushButton( i18n( "Resume All" ), this );
	connect(b7, SIGNAL(clicked()), this, SLOT(b7Pressed()));
      }
  }

  m_pLayout = new QVBoxLayout( this, KDialog::marginHint(),
			       KDialog::spacingHint() );
  m_pLayout->addStrut( 360 );	// makes dlg at least that wide

  // User tries to overwrite a file with itself ?
  if ( _mode & M_OVERWRITE_ITSELF ) {
      QLabel *lb = new QLabel( i18n("This action would overwrite '%1' with itself.\n"
                                    "Do you want to rename it instead?").arg(KStringHandler::csqueeze(src,100)), this );
      m_pLayout->addWidget(lb);
  }  else if ( _mode & M_OVERWRITE ) {
      QGridLayout * gridLayout = new QGridLayout( 0L, 9, 2, KDialog::marginHint(),
                                                  KDialog::spacingHint() );
      m_pLayout->addLayout(gridLayout);
      gridLayout->setColStretch(0,0);
      gridLayout->setColStretch(1,10);

      QString sentence1 = (mtimeDest < mtimeSrc)
                         ? i18n("An older item named '%1' already exists.")
                         : i18n("A newer item named '%1' already exists.");
      QLabel * lb1 = new QLabel( sentence1.arg(KStringHandler::csqueeze(dest,100)), this );
      gridLayout->addMultiCellWidget( lb1, 0, 0, 0, 1 ); // takes the complete first line

      lb1 = new QLabel( this );
      lb1->setPixmap( KMimeType::pixmapForURL( dest ) );
      gridLayout->addMultiCellWidget( lb1, 1, 3, 0, 0 ); // takes the first column on rows 1-3

      int row = 1;
      if ( sizeDest != (unsigned long)-1 )
      {
          QLabel * lb = new QLabel( i18n("size %1").arg( KIO::convertSize(sizeDest) ), this );
          gridLayout->addWidget( lb, row, 1 );
          row++;
      }
      if ( ctimeDest != (time_t)-1 )
      {
          QDateTime dctime; dctime.setTime_t( ctimeDest );
          QLabel * lb = new QLabel( i18n("created on %1").arg( KGlobal::locale()->formatDateTime(dctime) ), this );
          gridLayout->addWidget( lb, row, 1 );
          row++;
      }
      if ( mtimeDest != (time_t)-1 )
      {
          QDateTime dmtime; dmtime.setTime_t( mtimeDest );
          QLabel * lb = new QLabel( i18n("modified on %1").arg( KGlobal::locale()->formatDateTime(dmtime) ), this );
          gridLayout->addWidget( lb, row, 1 );
          row++;
      }

      // rows 1 to 3 are the details (size/ctime/mtime), row 4 is empty
      gridLayout->addRowSpacing( 4, 20 );

      QLabel * lb2 = new QLabel( i18n("The source file is '%1'").arg(src), this );
      gridLayout->addMultiCellWidget( lb2, 5, 5, 0, 1 ); // takes the complete first line

      lb2 = new QLabel( this );
      lb2->setPixmap( KMimeType::pixmapForURL( src ) );
      gridLayout->addMultiCellWidget( lb2, 6, 8, 0, 0 ); // takes the first column on rows 6-8

      row = 6;

      if ( sizeSrc != (unsigned long)-1 )
      {
          QLabel * lb = new QLabel( i18n("size %1").arg( KIO::convertSize(sizeSrc) ), this );
          gridLayout->addWidget( lb, row, 1 );
          row++;
      }
      if ( ctimeSrc != (time_t)-1 )
      {
          QDateTime dctime; dctime.setTime_t( ctimeSrc );
          QLabel * lb = new QLabel( i18n("created on %1").arg( KGlobal::locale()->formatDateTime(dctime) ), this );
          gridLayout->addWidget( lb, row, 1 );
          row++;
      }
      if ( mtimeSrc != (time_t)-1 )
      {
          QDateTime dmtime; dmtime.setTime_t( mtimeSrc );
          QLabel * lb = new QLabel( i18n("modified on %1").arg( KGlobal::locale()->formatDateTime(dmtime) ), this );
          gridLayout->addWidget( lb, row, 1 );
          row++;
      }
  } else
  {
      // I wonder when this happens (David). And 'dest' isn't shown at all here...
      QString sentence1;
      if (mtimeDest < mtimeSrc)
	  sentence1 = i18n("An older item than '%1' already exists.\n").arg(src);
      else
	  sentence1 = i18n("A newer item than '%1' already exists.\n").arg(src);

      QLabel *lb = new QLabel( sentence1 + i18n("Do you want to use another file name?"), this );
      m_pLayout->addWidget(lb);
  }

  m_pLineEdit = new QLineEdit( this );
  m_pLayout->addWidget( m_pLineEdit );
  m_pLineEdit->setText( KURL(dest).fileName() );
  if (b1)
      connect(m_pLineEdit, SIGNAL(textChanged(const QString &)),
              SLOT(enableRenameButton(const QString &)));

  m_pLayout->addSpacing( 10 );

  QHBoxLayout* layout = new QHBoxLayout();
  m_pLayout->addLayout( layout );

  layout->addStretch(1);

  if ( b1 )
    layout->addWidget( b1 );
  if ( b2 )
    layout->addWidget( b2 );
  if ( b3 )
    layout->addWidget( b3 );
  if ( b4 )
    layout->addWidget( b4 );
  if ( b5 )
    layout->addWidget( b5 );
  if ( b6 )
    layout->addWidget( b6 );
  if ( b7 )
    layout->addWidget( b7 );

  b0->setDefault( true );
  layout->addWidget( b0 );

  resize( sizeHint() );
}

RenameDlg::~RenameDlg()
{
}

void RenameDlg::enableRenameButton(const QString &newDest)
{
  if (newDest != dest)
  {
    b1->setEnabled(true);
    b1->setDefault(true);
  }
  else
    b1->setEnabled(false);
}

KURL RenameDlg::newDestURL()
{
  KURL newDest( dest );
  newDest.setFileName( m_pLineEdit->text() );
  return newDest;
}

void RenameDlg::b0Pressed()
{
  done( 0 );
}

// Rename
void RenameDlg::b1Pressed()
{
  if ( m_pLineEdit->text()  == "" )
    return;

  KURL u = newDestURL();
  if ( u.isMalformed() )
  {
    KMessageBox::error( this, i18n( "Malformed URL\n%1" ).arg( u.prettyURL() ) );
    return;
  }

  done( 1 );
}

void RenameDlg::b2Pressed()
{
  done( 2 );
}

void RenameDlg::b3Pressed()
{
  done( 3 );
}

void RenameDlg::b4Pressed()
{
  done( 4 );
}

void RenameDlg::b5Pressed()
{
  done( 5 );
}

void RenameDlg::b6Pressed()
{
  done( 6 );
}

void RenameDlg::b7Pressed()
{
  done( 7 );
}

RenameDlg_Result KIO::open_RenameDlg( const QString & _caption,
                                      const QString & _src, const QString & _dest,
                                      RenameDlg_Mode _mode,
                                      QString& _new,
                                      unsigned long sizeSrc,
                                      unsigned long sizeDest,
                                      time_t ctimeSrc,
                                      time_t ctimeDest,
                                      time_t mtimeSrc,
                                      time_t mtimeDest)
{
  Q_ASSERT(kapp);

  RenameDlg dlg( 0L, _caption, _src, _dest, _mode,
                 sizeSrc, sizeDest, ctimeSrc, ctimeDest, mtimeSrc, mtimeDest,
                 true /*modal*/ );
  int i = dlg.exec();
  _new = dlg.newDestURL().path();

  return (RenameDlg_Result)i;
}

#include "renamedlg.moc"

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

#include <kapp.h>
#include <kio/global.h>
#include <kdialog.h>
#include <klocale.h>
#include <kglobal.h>
#include <kurl.h>
#include <kprotocolmanager.h>

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
  src = _src;
  dest = _dest;

  b0 = b1 = b2 = b3 = b4 = b5 = b6 = b7 = 0L;

  setCaption( _caption );

  b0 = new QPushButton( i18n( "Cancel" ), this );
  connect(b0, SIGNAL(clicked()), this, SLOT(b0Pressed()));

  b1 = new QPushButton( i18n( "Rename" ), this );
  b1->setEnabled(false);
  connect(b1, SIGNAL(clicked()), this, SLOT(b1Pressed()));

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
  QLabel *lb;

  if ( _mode & M_OVERWRITE_ITSELF ) {
    lb = new QLabel( i18n("This action would overwrite '%1' with itself.\n"
			  "Do you want to rename it instead?").arg(src), this );
  }  else if ( _mode & M_OVERWRITE ) {
      QString sentence1;

      // TODO rewrite this with a GridLayout and even icons, a la kpropsdlg
      if (mtimeDest < mtimeSrc)
          // The url is last since it may contain '%'
	  sentence1 = i18n("An older item named '%4' already exists\n"
                           "    size %1\n    created on %2\n    modified on %3\n");
      else
	  sentence1 = i18n("A newer item named '%4' already exists\n"
                           "    size %1\n    created on %2\n    modified on %3\n");

      QDateTime dctime; dctime.setTime_t( ctimeDest );
      QDateTime dmtime; dmtime.setTime_t( mtimeDest );

      sentence1 = sentence1.
        arg(sizeDest == (unsigned long)-1 ? i18n("unknown") : KIO::convertSize(sizeDest)).
        arg(ctimeDest == (time_t)-1 ? i18n("unknown") : KGlobal::locale()->formatDateTime(dctime)).
        arg(mtimeDest == (time_t)-1 ? i18n("unknown") : KGlobal::locale()->formatDateTime(dmtime)).
        arg(dest);

      QString sentence2 = i18n("The original file is '%4'\n"
                               "    size %1\n    created on %2\n    modified on %3\n");
      dctime.setTime_t( ctimeSrc );
      dmtime.setTime_t( mtimeSrc );
      sentence2 = sentence2.
        arg(sizeSrc == (unsigned long)-1 ? i18n("unknown") : KIO::convertSize(sizeSrc)).
        arg(ctimeSrc == (time_t)-1 ? i18n("unknown") : KGlobal::locale()->formatDateTime(dctime)).
        arg(mtimeSrc == (time_t)-1 ? i18n("unknown") : KGlobal::locale()->formatDateTime(dmtime)).
        arg(src);

      lb = new QLabel( sentence1 + "\n" + sentence2, this );
  } else
  {
      // I wonder when this happens (David). And 'dest' isn't shown at all here...
      QString sentence1;
      if (mtimeDest < mtimeSrc)
	  sentence1 = i18n("An older item than '%1' already exists.\n").arg(src);
      else
	  sentence1 = i18n("A newer item than '%1' already exists.\n").arg(src);

      lb = new QLabel( sentence1 + i18n("Do you want to use another file name?"), this );
  }

  m_pLayout->addWidget(lb);
  m_pLineEdit = new QLineEdit( this );
  m_pLayout->addWidget( m_pLineEdit );
  m_pLineEdit->setText( KURL(dest).fileName() );
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
  ASSERT(kapp);

  RenameDlg dlg( 0L, _caption, _src, _dest, _mode,
                 sizeSrc, sizeDest, ctimeSrc, ctimeDest, mtimeSrc, mtimeDest,
                 true /*modal*/ );
  int i = dlg.exec();
  _new = dlg.newDestURL().path();

  return (RenameDlg_Result)i;
}

/*
unsigned long KIO::getOffset( QString dest ) {

  if ( KProtocolManager::self().markPartial() )
    dest += ".part";

  KURL d( dest );
  QFileInfo info;
  info.setFile( d.path() );

  return info.size();
}
*/

#include "renamedlg.moc"

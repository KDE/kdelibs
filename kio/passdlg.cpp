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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kio/passdlg.h"

#include <string.h>

#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qaccel.h>

#include <kbuttonbox.h>
#include <klocale.h>
#include <kapp.h>

using namespace KIO;

PassDlg::PassDlg( QWidget* parent, const char* name, bool modal,
		          WFlags wflags, const QString& _head,
		          const QString& _user, const QString& _pass )
        : KDialog(parent, name, modal, wflags)
{

   QVBoxLayout *layout = new QVBoxLayout( this, marginHint(), spacingHint() );
   //
   // Bei Bedarf einen kleinen Kommentar als Label einfuegen
   // NB20000703: Hmm??? Something about pretty comments? Made the change
   //             anyway, now the slave decides about the whole string.
   QString msg;
   if ( !_head.isEmpty() )
   {
        msg = "<qt><center><nobr>" + _head + "</nobr></center></qt>";

   }
   else
   {
        msg = i18n( "Enter your user name and password." );
   }

   QLabel *l = new QLabel(msg, this);
   layout->addWidget( l );

   QGridLayout *grid = new QGridLayout( 3, 5 );
   layout->addLayout( grid );

   l = new QLabel( i18n( "Username:" ), this );
   l->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   grid->addWidget( l, 0, 1 );
   m_pUser = new QLineEdit( this );
   grid->addWidget( m_pUser, 0, 3 );
   l->setBuddy( m_pUser );

   l = new QLabel( i18n( "Password:" ), this );
   l->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   grid->addWidget( l, 2, 1 );
   m_pPass = new QLineEdit( this );
   m_pPass->setEchoMode( QLineEdit::Password );
   grid->addWidget( m_pPass, 2, 3 );
   l->setBuddy( m_pPass );

   if ( !_user.isEmpty() )
     m_pUser->setText( _user );

   if ( !_pass.isEmpty() )
     m_pPass->setText( _pass );

   layout->addSpacing( spacingHint() );

   // Connect vom LineEdit herstellen und Accelerator
   QAccel *ac = new QAccel(this);
   ac->connectItem( ac->insertItem(Key_Escape), this, SLOT(reject()) );

   connect( m_pPass, SIGNAL(returnPressed()), SLOT(accept()) );

   // Die Buttons "OK" & "Cancel" erzeugen
   KButtonBox *bbox = new KButtonBox(this);
   layout->addWidget(bbox);

   bbox->addStretch();
   QPushButton *b1, *b2;
   b1 = bbox->addButton(i18n("OK"));
   b2 = bbox->addButton(i18n("Cancel"));
   bbox->layout();
       
   // Buttons mit Funktionaliataet belegen
   connect( b1, SIGNAL(clicked()), SLOT(accept()) );
   connect( b2, SIGNAL(clicked()), SLOT(reject()) );

   // Fenstertitel
   setCaption( i18n("Authorization Request") );

   // Focus
   if ( _user.isEmpty() )
     m_pUser->setFocus();
   else
     m_pPass->setFocus();

   layout->addStretch( 10 );

   resize( sizeHint() );
}

void PassDlg::setEnableUserField( bool enable, bool gainsFocus )
{
  m_pUser->setEnabled(enable);
  if( enable && gainsFocus )
    m_pUser->setFocus();

  if( !enable && m_pUser->hasFocus() )
    m_pPass->setFocus();
}

#include "passdlg.moc"

/* This file is part of the KDE project
 *
 * Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "khtml_find.h"
#include "khtml_part.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <kstringhandler.h>
#include <qhbox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qvbuttongroup.h>
#include <kregexpeditorinterface.h>
#include <kparts/componentfactory.h>
#include <assert.h>

KHTMLFind::KHTMLFind( KHTMLPart *part, QWidget *parent, const char *name )
  : KEdFind( parent, name, false ), m_editorDialog(0)
{
//  connect( this, SIGNAL( done() ),
//           this, SLOT( slotDone() ) );
  connect( this, SIGNAL( search() ),
	   this, SLOT( slotSearch() ) );
  m_first = true;
  m_part = part;
  m_found = false;

  QHBox* row = new QHBox( group );
  m_asRegExp = new QCheckBox( i18n("As &Regular Expression"), row, "asRegexp" );

  if (!KTrader::self()->query("KRegExpEditor/KRegExpEditor").isEmpty())
  {
      m_editRegExp = new QPushButton( i18n("&Edit..."), row, "editRegExp" );

      connect( m_asRegExp, SIGNAL( toggled(bool) ), m_editRegExp, SLOT( setEnabled(bool) ) );
      connect( m_editRegExp, SIGNAL( clicked() ), this, SLOT( slotEditRegExp() ) );
      m_editRegExp->setEnabled( false );
  }
  else
  {
      m_editRegExp = 0;
  }
}

KHTMLFind::~KHTMLFind()
{
}

void KHTMLFind::slotDone()
{
  accept();
}

void KHTMLFind::slotSearch()
{
  if ( m_first )
  {
    m_part->findTextBegin();
    m_first = false;
  }

  bool forward = !get_direction();

  if ( m_part->findTextNext( getText(), forward, case_sensitive(),
	                     m_asRegExp->isChecked() ) )
    m_found = true;
  else if ( m_found )
  {
    if ( forward )
    {
      if ( KMessageBox::questionYesNo( this,
           i18n("End of document reached.\n"\
                "Continue from the beginning?"),
	   i18n("Find") ) == KMessageBox::Yes )
      {
        m_first = true;
	slotSearch();
      }
    }
    else
    {
      if ( KMessageBox::questionYesNo( this,
           i18n("Beginning of document reached.\n"\
                "Continue from the end?"),
	   i18n("Find") ) == KMessageBox::Yes )
      {
        m_first = true;
	slotSearch();
      }
    }
  }
  else
    KMessageBox::information( this, 
    	i18n( "Search string '%1' not found." ).arg(KStringHandler::csqueeze(getText())),
	i18n( "Find" ) );
}

void KHTMLFind::setNewSearch()
{
  m_first = true;
  m_found = false;
}

void KHTMLFind::slotEditRegExp()
{
  if ( m_editorDialog == 0 )
      m_editorDialog = KParts::ComponentFactory::createInstanceFromQuery<QDialog>( "KRegExpEditor/KRegExpEditor", QString::null, this );

  assert( m_editorDialog );

  KRegExpEditorInterface *iface = static_cast<KRegExpEditorInterface *>( m_editorDialog->qt_cast( "KRegExpEditorInterface" ) );
  assert( iface );

  iface->setRegExp( getText() );
  bool ret = m_editorDialog->exec();
  if ( ret == QDialog::Accepted)
    setText( iface->regExp() );
}

#include "khtml_find.moc"

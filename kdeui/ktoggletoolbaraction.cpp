/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2002 Joseph Wenninger <jowenn@kde.org>
              (C) 2003 Andras Mantia <amantia@kde.org>
              (C) 2005-2006 Hamish Rodda <rodda@kde.org>

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

#include "ktoggletoolbaraction.h"

#include <QEvent>

#include <kdebug.h>
#include <klocale.h>

#include "kmainwindow.h"
#include "ktoolbar.h"

// BEGIN KToggleToolBarAction
KToggleToolBarAction::KToggleToolBarAction( const char* toolBarName,
         const QString& text, KActionCollection* parent, const QString& name )
  : KToggleAction( text, parent, name )
  , m_toolBarName( toolBarName )
  , m_toolBar( 0L )
  , m_beingToggled( false )
  , d(0L)
{
}

KToggleToolBarAction::KToggleToolBarAction( KToolBar *toolBar, const QString &text,
                                            KActionCollection *parent, const char *name )
  : KToggleAction( text, parent, name )
  , m_toolBarName( 0 ), m_toolBar( toolBar ), m_beingToggled( false )
  , d(0L)
{
    m_toolBar->installEventFilter(this);

    m_beingToggled = true;
    if (m_toolBar->isVisible())
      setChecked(true);
    m_beingToggled = false;
}

KToggleToolBarAction::~KToggleToolBarAction()
{
    //delete d;
}

bool KToggleToolBarAction::eventFilter( QObject * watched, QEvent * event )
{
    if (m_beingToggled)
        return false;

    m_beingToggled = true;

    if (watched == m_toolBar) {
        switch (event->type()) {
            case QEvent::Hide:
                if (isChecked())
                    setChecked(false);
                break;

            case QEvent::Show:
                if (!isChecked())
                    setChecked(true);
                break;

            default:
                break;
        }
    }

    m_beingToggled = false;

    return false;
}

KToolBar * KToggleToolBarAction::toolBar( )
{
  return m_toolBar;
}

void KToggleToolBarAction::slotToggled( bool checked )
{
  if (!m_beingToggled && m_toolBar && checked != m_toolBar->isVisible() ) {
    m_beingToggled = true;

    if (checked) {
      m_toolBar->show();
    } else {
      m_toolBar->hide();
    }

    m_beingToggled = false;

    QMainWindow* mw = m_toolBar->mainWindow();
    if ( mw && qobject_cast<KMainWindow*>( mw ) )
      static_cast<KMainWindow *>( mw )->setSettingsDirty();
  }

  KToggleAction::slotToggled(checked);
}

/* vim: et sw=2 ts=2
 */

#include "ktoggletoolbaraction.moc"

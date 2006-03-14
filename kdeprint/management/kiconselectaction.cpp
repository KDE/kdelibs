/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kiconselectaction.h"

KIconSelectAction::KIconSelectAction(const QString& text, KActionCollection* parent, const char* name)
  : KSelectAction(text, parent, name)
  , d(0L)
{
}

KIconSelectAction::~KIconSelectAction()
{
	//delete d;
}

void KIconSelectAction::setItems(const QStringList& lst, const QStringList& iconlst)
{
  clear();

  Q_ASSERT(iconlst.count() >= lst.count());

  for (int i = 0; i < lst.count(); ++i) {
    if ( !lst.at(i).isEmpty() ) {
      KAction* action = new KAction(iconlst.at(i), lst.at(i), parentCollection(), 0);
      action->setShortcutConfigurable(false);
      addAction(action);

    } else {
      QAction* action = new QAction(this);
      action->setSeparator(true);
      addAction(action);
    }
  }

  // Disable if empty and not editable
  setEnabled( lst.count() > 0 || isEditable() );
}

#include "kiconselectaction.moc"

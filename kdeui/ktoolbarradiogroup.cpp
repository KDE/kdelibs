/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
              (C) 1997, 1998 Mark Donohoe (donohoe@kde.org)
              (C) 1997, 1998 Sven Radej (radej@kde.org)
              (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)
			  (C) 1999 Chris Schlaeger (cs@kde.org)

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

// $Id$
// $Log$

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ktoolbarradiogroup.h"
#include "ktoolbar.h"
#include "ktoolbaritem.h"
#include "ktoolbarbutton.h"

/*************************************************************************
 *                          KToolBarRadioGroup                                  *
 *************************************************************************/


KToolBarRadioGroup::KToolBarRadioGroup (KToolBar *_parent, const char *_name)
: QObject(_parent, _name)
{
  buttons = new KToolBarButtonList();
  tb = _parent;
  connect (tb, SIGNAL(toggled(int)), this, SLOT(slotToggled(int)));
}

KToolBarRadioGroup::~KToolBarRadioGroup()
{
  delete buttons;
}

void KToolBarRadioGroup::addButton (int id)
{
  for (KToolBarItem *b = tb->items->first(); b; b=tb->items->next())
    if (b->ID() == id )
    {
      buttons->insert(id, (KToolBarButton *) b->getItem());
      ((KToolBarButton *) b->getItem())->setRadio(true);
    }
}

void KToolBarRadioGroup::removeButton (int id)
{
  if (!buttons->find(id))
     return;
  buttons->find(id)->setRadio(false);
  buttons->remove(id);
}

void KToolBarRadioGroup::slotToggled(int id)
{
  if (buttons->find(id) && buttons->find(id)->isOn())
  {
    QIntDictIterator<KToolBarButton> it(*buttons);
    while (it.current())
    {
      if (it.currentKey() != id)
        it.current()->on(false);
      ++it;
    }
  }
}

#include "ktoolbarradiogroup.moc"


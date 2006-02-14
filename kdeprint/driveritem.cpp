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

#include "driveritem.h"
#include "driver.h"

#include <qpainter.h>
#include <kiconloader.h>
#include <kdebug.h>

DriverItem::DriverItem(QTreeWidget *parent, DrBase *item)
: QTreeWidgetItem(parent), m_item(item), m_conflict(false)
{
	setIcon(0,SmallIcon("fileprint"));
	updateText();
}

DriverItem::DriverItem(QTreeWidgetItem *parent, QTreeWidgetItem *after, DrBase *item)
: QTreeWidgetItem(parent, after), m_item(item), m_conflict(false)
{
	if (item) setIcon(0,SmallIcon((item->isOption() ? "document" : "folder")));
	updateText();
}

void DriverItem::updateText()
{
	if (m_item)
	{
		QString	s(m_item->get("text"));
		if (m_item->isOption())
			s.append(QString::fromLatin1(": <%1>").arg(m_item->prettyText()));
		if (m_item->type() == DrBase::List)
		{
			// remove all children: something has changed (otherwise this
			// function would not be called), so it make sense to remove
			// those children in all cases.
			while (child(0))
				delete child(0);
			DrBase	*ch = static_cast<DrListOption*>(m_item)->currentChoice();
			if (ch && ch->type() == DrBase::ChoiceGroup)
			{
				// add new children
				static_cast<DrChoiceGroup*>(ch)->createItem(this);
			}
		}
		setText(0,s);
	}
	else
		setText(0,"ERROR");
}

bool DriverItem::updateConflict()
{
	m_conflict = false;
	if (m_item)
	{
		if (!m_item->isOption())
		{
      for ( int i = 0; i < childCount(); ++i ) {
			  DriverItem	*item = (DriverItem*)child(i);
				if (item->updateConflict())
					m_conflict = true;
      }
		}
		else
		{
			m_conflict = (m_item->conflict());
		}
	}
	return m_conflict;
}

void DriverItem::updateTextRecursive()
{
	if ( m_item->isOption() )
		updateText();

  for ( int i = 0; i < childCount(); ++i ) {
    DriverItem	*item = (DriverItem*)child(i);
    item->updateTextRecursive();
  }
}

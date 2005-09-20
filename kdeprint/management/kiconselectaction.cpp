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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "kiconselectaction.h"

#include <q3popupmenu.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <ktoolbar.h>
#include <ktoolbarbutton.h>

class KIconSelectActionPrivate
{
public:
	KIconSelectActionPrivate()
	{
		m_menu = 0;
	}
	QStringList	m_iconlst;
	QMenu*	m_menu;
};

KIconSelectAction::KIconSelectAction(const QString& text, int accel, QObject* parent, const char* name)
: KSelectAction(text, accel, parent, name)
{
	d = new KIconSelectActionPrivate;
}

KIconSelectAction::~KIconSelectAction()
{
	delete d;
}

void KIconSelectAction::updateIcons()
{
	if (d->m_menu)
	{
		QStringList	lst = items();
		for (int id=0; id<lst.count(); ++id)
			d->m_menu->changeItem(id, SmallIconSet(d->m_iconlst[id]), lst[id]);
	}
}

void KIconSelectAction::createPopupMenu()
{
	if (!d->m_menu)
	{
		d->m_menu = popupMenu();
		updateIcons();
	}
}

void KIconSelectAction::setItems(const QStringList& lst, const QStringList& iconlst)
{
	KSelectAction::setItems(lst);
	d->m_iconlst = iconlst;
	updateIcons();
}

int KIconSelectAction::plug(QWidget* widget, int index)
{
	int	value(-1);
	if (widget->inherits("QPopupMenu"))
	{
		createPopupMenu();
		value = KSelectAction::plug(widget, index);
	}
	else if (widget->inherits("KToolBar"))
	{
		KToolBar* bar = static_cast<KToolBar*>(widget);
		int id = KAction::getToolButtonID();
		// To have a correct layout in the toolbar, a non
		// empty icon has to be used. Use "unknown" by default.
		QString	iconName = (currentItem() != -1 ? d->m_iconlst[currentItem()] : "unknown");

		createPopupMenu();
		bar->insertButton(iconName, id, true, plainText(), index);
		bar->getButton(id)->setPopup(d->m_menu, true);
		bar->setItemEnabled(id, isEnabled());
		addContainer(bar, id);
		connect(bar, SIGNAL(destroyed()), SLOT(slotDestroyed()));

		value = containerCount()-1;
	}
	return value;
}

void KIconSelectAction::updateCurrentItem(int id)
{
	QWidget*	w = container(id);
	if (w->inherits("KToolBar"))
		static_cast<KToolBar*>(w)->getButton(itemId(id))->setIcon(d->m_iconlst[currentItem()]);
	else
		KSelectAction::updateCurrentItem(id);
}

void KIconSelectAction::setCurrentItem(int index)
{
	KSelectAction::setCurrentItem(index);
}

#include "kiconselectaction.moc"

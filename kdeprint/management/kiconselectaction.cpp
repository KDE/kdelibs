/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
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

#include <qpopupmenu.h>
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
	QPopupMenu*	m_menu;
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

		createPopupMenu();
		bar->insertButton(QString::null, id, true, plainText(), index);
		bar->getButton(id)->setPopup(d->m_menu, true);
		if (currentItem() != -1)
			bar->getButton(id)->setIcon(d->m_iconlst[currentItem()]);
		bar->setItemEnabled(id, isEnabled());
		addContainer(bar, id);
		connect(bar, SIGNAL(destroyed()), SLOT(slotDestroyed()));

		value = containerCount()-1;
	}
	return value;
}

void KIconSelectAction::setCurrentItem(int id, int index)
{
	QWidget*	w = container(id);
	if (w->inherits("KToolBar"))
		static_cast<KToolBar*>(w)->getButton(itemId(id))->setIcon(d->m_iconlst[index]);
	else
		KSelectAction::setCurrentItem(id, index);
}

void KIconSelectAction::setCurrentItem(int index)
{
	KSelectAction::setCurrentItem(index);
}

#include "kiconselectaction.moc"

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

#include "kprintaction.h"

#include <kprinter.h>
#include <kdeprint/kmmanager.h>
#include <kiconloader.h>
#include <kmenu.h>
#include <klocale.h>

class KPrintAction::KPrintActionPrivate
{
public:
	KPrintActionPrivate()
	{
		type = All;
		parentWidget = 0;
	}

	PrinterType	type;
	QStringList	printers;
	QWidget *parentWidget;
};

KPrintAction::KPrintAction(const QString& text, PrinterType type, QWidget *parentWidget, KActionCollection  *parent, const char *name)
: KActionMenu(text, parent, name),d(new KPrintActionPrivate())
{
	initialize(type, parentWidget);
}

KPrintAction::KPrintAction(const QString& text, const QIcon& icon, PrinterType type, QWidget *parentWidget, KActionCollection *parent, const char 
*name)
: KActionMenu(text, icon, parent, name),d(new KPrintActionPrivate())
{
	initialize(type, parentWidget);
}

KPrintAction::KPrintAction(const QString& text, const QString& icon, PrinterType type, QWidget *parentWidget, KActionCollection *parent, const char *name)
: KActionMenu(text, icon, parent, name),d(new KPrintActionPrivate())
{
	initialize(type, parentWidget);
}

KPrintAction::~KPrintAction()
{
	delete d;
}

void KPrintAction::initialize(PrinterType type, QWidget *parentWidget)
{
	connect(popupMenu(), SIGNAL(aboutToShow()), SLOT(slotAboutToShow()));
	connect(popupMenu(), SIGNAL(activated(int)), SLOT(slotActivated(int)));

	d->type = type;
	d->parentWidget = parentWidget;
}

void KPrintAction::slotAboutToShow()
{
	popupMenu()->clear();
	d->printers.clear();
	QList<KMPrinter*>	*prts = KMManager::self()->printerList();
	if (prts && !prts->isEmpty())
	{
		QListIterator<KMPrinter*>	it(*prts);
		bool	first(false);
		int	ID(0);
		while (it.hasNext())
		{
      KMPrinter *printer(it.next());
			if (d->type == All || (d->type == Specials && printer->isSpecial()) || (d->type == Regular && !printer->isSpecial()))
			{
				if (d->type == All && !first && printer->isSpecial())
				{
					if (popupMenu()->actions().count() > 0)
						popupMenu()->addSeparator();
					first = true;
				}
				popupMenu()->insertItem(SmallIconSet(printer->pixmap()), printer->name(), ID++);
				d->printers.append(printer->name());
			}
		}
	}
}

void KPrintAction::slotActivated(int ID)
{
	KPrinter	printer(false);
	KMPrinter	*mprt = KMManager::self()->findPrinter(d->printers[ID]);
	if (mprt && mprt->autoConfigure(&printer, d->parentWidget))
	{
		// emit the signal
		emit print(&printer);
	}
}

KPrintAction* KPrintAction::exportAll(QWidget *parentWidget, KActionCollection *parent, const char *name)
{
	return new KPrintAction(i18n("&Export..."), All, parentWidget, parent, (name ? name : "export_all"));
}

KPrintAction* KPrintAction::exportRegular(QWidget *parentWidget, KActionCollection *parent, const char *name)
{
	return new KPrintAction(i18n("&Export..."), Regular, parentWidget, parent, (name ? name : "export_regular"));
}

KPrintAction* KPrintAction::exportSpecial(QWidget *parentWidget, KActionCollection *parent, const char *name)
{
	return new KPrintAction(i18n("&Export..."), Specials, parentWidget, parent, (name ? name : "export_special"));
}

#include "kprintaction.moc"

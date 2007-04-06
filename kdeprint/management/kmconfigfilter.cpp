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

#include "kmconfigfilter.h"
#include "kmmanager.h"
#include "kmfactory.h"

#include <QGroupBox>
#include <QVBoxLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QToolButton>
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QApplication>

#include <klocale.h>
#include <kconfiggroup.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <klistwidget.h>
#include <kdialog.h>

KMConfigFilter::KMConfigFilter(QWidget *parent)
: KMConfigPage(parent)
{
	setPageName(i18n("Filter"));
	setPageHeader(i18n("Printer Filtering Settings"));
	setPagePixmap("search-filter");

	QGroupBox	*box = new QGroupBox(i18n("Printer Filter"), this);
	box->setLayout( new QVBoxLayout );

	m_list1 = new KListWidget(box);
	m_list1->setSelectionMode(KListWidget::ExtendedSelection);
	m_list2 = new KListWidget(box);
	m_list2->setSelectionMode(KListWidget::ExtendedSelection);
	m_add = new QToolButton( box );
	m_add->setIcon(QApplication::isRightToLeft() ? KIcon( "go-previous" ) : KIcon( "go-next" ));
	m_remove = new QToolButton( box );
	m_remove->setIcon(QApplication::isRightToLeft() ? KIcon( "go-next" ) : KIcon( "go-previous" ));
	m_locationre = new QLineEdit(box);
	QLabel	*lab = new QLabel(box);
	lab->setText(i18n("The printer filtering allows you to view only a specific set of "
	                  "printers instead of all of them. This may be useful when there are a "
			  "lot of printers available but you only use a few ones. Select the "
			  "printers you want to see from the list on the left or enter a <b>Location</b> "
			  "filter (ex: Group_1*). Both are cumulative and ignored if empty."));
	lab->setTextFormat(Qt::RichText);
	lab->setWordWrap(true);
	QLabel	*lab1 = new QLabel(i18n("Location filter:"), box);

	QVBoxLayout	*l0 = new QVBoxLayout(this);
	l0->setMargin(0);
	l0->setSpacing(KDialog::spacingHint());
	l0->addWidget(box, 1);
	QVBoxLayout	*l1 = new QVBoxLayout();
  l1->setSpacing(KDialog::spacingHint());
  box->layout()->addItem(l1);
	l1->addWidget(lab);
	QGridLayout	*l2 = new QGridLayout();
    l2->setMargin(0);
    l2->setSpacing(KDialog::spacingHint());
	l1->addLayout(l2);
	l2->setRowStretch(0, 1);
	l2->setRowStretch(3, 1);
	l2->setColumnStretch(0, 1);
	l2->setColumnStretch(2, 1);
    l2->addWidget(m_list1, 0, 0, 4, 1);
    l2->addWidget(m_list2, 0, 2, 4, 1);
	l2->addWidget(m_add, 1, 1);
	l2->addWidget(m_remove, 2, 1);
	QHBoxLayout	*l3 = new QHBoxLayout();
  l3->setSpacing(KDialog::spacingHint());
	l1->addLayout(l3, 0);
	l3->addWidget(lab1, 0);
	l3->addWidget(m_locationre, 1);

	connect(m_add, SIGNAL(clicked()), SLOT(slotAddClicked()));
	connect(m_remove, SIGNAL(clicked()), SLOT(slotRemoveClicked()));
	connect(m_list1, SIGNAL(selectionChanged()), SLOT(slotSelectionChanged()));
	connect(m_list2, SIGNAL(selectionChanged()), SLOT(slotSelectionChanged()));
	m_add->setEnabled(false);
	m_remove->setEnabled(false);
}

void KMConfigFilter::loadConfig(KConfig *conf)
{
	KConfigGroup cg( conf, "Filter");
	QStringList	m_plist = cg.readEntry("Printers", QStringList());
	QListIterator<KMPrinter*>	it(KMManager::self()->printerListComplete(false));
	while (it.hasNext())
	{
    KMPrinter *printer(it.next());
		if (!printer->isSpecial() && !printer->isVirtual())
		{
			KListWidget *lb = (m_plist.contains(printer->printerName()) ? m_list2 : m_list1);
			
            QListWidgetItem* item = new QListWidgetItem();
            item->setIcon(SmallIcon(printer->pixmap()));
            item->setText(printer->printerName());
		    lb->addItem(item);
        }
	}
	m_list1->model()->sort(0);
	m_list2->model()->sort(0);
	m_locationre->setText(cg.readEntry("LocationRe"));
}

void KMConfigFilter::saveConfig(KConfig *conf)
{
	KConfigGroup cg( conf, "Filter");
	QStringList	plist;
	for (int i=0; i<m_list2->count(); i++)
		plist << m_list2->item(i)->text();
	cg.writeEntry("Printers", plist);
	cg.writeEntry("LocationRe", m_locationre->text());
}

void KMConfigFilter::transfer(KListWidget *from, KListWidget *to)
{
	for (int i=0; i<from->count();)
	{
		if (from->item(i)->isSelected())
		{
			to->addItem(from->item(i)->clone());
			delete from->takeItem(i);
		}
		else
			i++;
	}
	to->model()->sort(0);
}

void KMConfigFilter::slotAddClicked()
{
	transfer(m_list1, m_list2);
}

void KMConfigFilter::slotRemoveClicked()
{
	transfer(m_list2, m_list1);
}

void KMConfigFilter::slotSelectionChanged()
{
	const KListWidget	*lb = static_cast<const KListWidget*>(sender());
	if (!lb)
		return;
	QToolButton	*pb = (lb == m_list1 ? m_add : m_remove);
	for (int i=0; i<lb->count(); i++)
		if (lb->item(i)->isSelected())
		{
			pb->setEnabled(true);
			return;
		}
	pb->setEnabled(false);
}

#include "kmconfigfilter.moc"

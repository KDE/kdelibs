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

#include "kmconfigfonts.h"

#include <klistview.h>
#include <qgroupbox.h>
#include <kpushbutton.h>
#include <qlayout.h>
#include <qheader.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qsettings.h>
#include <qwhatsthis.h>
#include <klocale.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kurlrequester.h>
#include <kfile.h>

KMConfigFonts::KMConfigFonts(QWidget *parent, const char *name)
: KMConfigPage(parent, name)
{
	setPageName(i18n("Fonts"));
	setPageHeader(i18n("Fonts Settings"));
	setPagePixmap("fonts");

	QGroupBox	*box = new QGroupBox(0, Qt::Vertical, i18n("Fonts Embedding"), this);
	QGroupBox	*box2 = new QGroupBox(0, Qt::Vertical, i18n("Fonts Path"), this);

	m_embedfonts = new QCheckBox(i18n("&Embed fonts in PostScript data when printing"), box);
	m_fontpath = new KListView(box2);
	m_fontpath->addColumn("");
	m_fontpath->header()->setStretchEnabled(true, 0);
	m_fontpath->header()->hide();
	m_fontpath->setSorting(-1);
	m_addpath = new KURLRequester(box2);
	m_addpath->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);
	m_up = new KPushButton(KGuiItem(i18n("&Up"), "up"), box2);
	m_down = new KPushButton(KGuiItem(i18n("&Down"), "down"), box2);
	m_add = new KPushButton(KGuiItem(i18n("&Add"), "add"), box2);
	m_remove = new KPushButton(KGuiItem(i18n("&Remove"), "editdelete"), box2);
	QLabel	*lab0 = new QLabel(i18n("Additional director&y:"), box2);
	lab0->setBuddy(m_addpath);

	QVBoxLayout	*l0 = new QVBoxLayout(box->layout(), 0);
	l0->addWidget(m_embedfonts);
	QVBoxLayout	*l1 = new QVBoxLayout(box2->layout(), 5);
	l1->addWidget(m_fontpath);
	QHBoxLayout	*l2 = new QHBoxLayout(0, 0, 10);
	l1->addLayout(l2);
	l2->addWidget(m_up);
	l2->addWidget(m_down);
	l2->addWidget(m_remove);
	l1->addSpacing(10);
	l1->addWidget(lab0);
	l1->addWidget(m_addpath);
	QHBoxLayout	*l3 = new QHBoxLayout(0, 0, 10);
	l1->addLayout(l3);
	l3->addStretch(1);
	l3->addWidget(m_add);
	QVBoxLayout	*l4 = new QVBoxLayout(this, 5, 10);
	l4->addWidget(box);
	l4->addWidget(box2);

	QWhatsThis::add(m_embedfonts,
			i18n("These options will automatically put fonts in the PostScript file "
                             "which are not present on the printer. Font embedding usually produces better print results "
			     "(closer to what you see on the screen), but larger print data as well."));
	QWhatsThis::add(m_fontpath, 
			i18n("When using font embedding you can select additional directories where "
			     "KDE should search for embeddable font files. By default, the X server "
			     "font path is used, so adding those directories is not needed. The default "
			     "search path should be sufficient in most cases."));

	connect(m_remove, SIGNAL(clicked()), SLOT(slotRemove()));
	connect(m_add, SIGNAL(clicked()), SLOT(slotAdd()));
	connect(m_up, SIGNAL(clicked()), SLOT(slotUp()));
	connect(m_down, SIGNAL(clicked()), SLOT(slotDown()));
	connect(m_fontpath, SIGNAL(selectionChanged()), SLOT(slotSelected()));
	connect(m_addpath, SIGNAL(textChanged(const QString&)), SLOT(slotTextChanged(const QString&)));
	m_add->setEnabled(false);
	m_remove->setEnabled(false);
	m_up->setEnabled(false);
	m_down->setEnabled(false);
}

void KMConfigFonts::loadConfig(KConfig *)
{
	QSettings	settings;
	m_embedfonts->setChecked(settings.readBoolEntry("/qt/embedFonts"));
	QStringList	paths = settings.readListEntry("/qt/fontPath", ':');
	QListViewItem	*item(0);
	for (QStringList::ConstIterator it=paths.begin(); it!=paths.end(); ++it)
		item = new QListViewItem(m_fontpath, item, *it);
}

void KMConfigFonts::saveConfig(KConfig *)
{
	QSettings	settings;
	settings.writeEntry("/qt/embedFonts", m_embedfonts->isChecked());
	QStringList	l;
	QListViewItem	*item = m_fontpath->firstChild();
	while (item)
	{
		l << item->text(0);
		item = item->nextSibling();
	}
	settings.writeEntry("/qt/fontPath", l, ':');
}

void KMConfigFonts::slotSelected()
{
	QListViewItem	*item = m_fontpath->selectedItem();
	m_remove->setEnabled(item);
	m_up->setEnabled(item && item->itemAbove());
	m_down->setEnabled(item && item->itemBelow());
}

void KMConfigFonts::slotAdd()
{
	if (m_addpath->url().isEmpty())
		return;
	QListViewItem	*lastItem(m_fontpath->firstChild());
	while (lastItem && lastItem->nextSibling())
		lastItem = lastItem->nextSibling();
	QListViewItem	*item = new QListViewItem(m_fontpath, lastItem, m_addpath->url());
	m_fontpath->setSelected(item, true);
}

void KMConfigFonts::slotRemove()
{
	delete m_fontpath->selectedItem();
	if (m_fontpath->currentItem())
		m_fontpath->setSelected(m_fontpath->currentItem(), true);
	slotSelected();
}

void KMConfigFonts::slotUp()
{
	QListViewItem	*citem = m_fontpath->selectedItem(), *nitem(0);
	if (!citem || !citem->itemAbove())
		return;
	nitem = new QListViewItem(m_fontpath, citem->itemAbove()->itemAbove(), citem->text(0));
	delete citem;
	m_fontpath->setSelected(nitem, true);
}

void KMConfigFonts::slotDown()
{
	QListViewItem	*citem = m_fontpath->selectedItem(), *nitem(0);
	if (!citem || !citem->itemBelow())
		return;
	nitem = new QListViewItem(m_fontpath, citem->itemBelow(), citem->text(0));
	delete citem;
	m_fontpath->setSelected(nitem, true);
}

void KMConfigFonts::slotTextChanged(const QString& t)
{
	m_add->setEnabled(!t.isEmpty());
}

#include "kmconfigfonts.moc"

/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  $Id$
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

#include "kminstancepage.h"
#include "kmprinter.h"
#include "kmfactory.h"
#include "kmvirtualmanager.h"
#include "kmmanager.h"
#include "kprinterpropertydialog.h"
#include "kprinter.h"
#include "kmtimer.h"

#include <qlayout.h>
#include <qregexp.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <kmessagebox.h>
#include <klineeditdlg.h>
#include <klistbox.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kdebug.h>

KMInstancePage::KMInstancePage(QWidget *parent, const char *name)
: QWidget(parent,name)
{
	m_view = new KListBox(this);
	m_printer = 0;

	initActions();

	QHBoxLayout	*main_ = new QHBoxLayout(this, 0, 0);
	main_->addWidget(m_view);
	QVBoxLayout	*sub_ = new QVBoxLayout(0, 0, 0);
	main_->addLayout(sub_);
	for (QValueList<QButton*>::Iterator it=m_buttons.begin(); it!=m_buttons.end(); ++it)
		if (*it)
			sub_->addWidget(*it);
		else
			sub_->addSpacing(10);
	sub_->addStretch(1);

	QWhatsThis::add(this,
		i18n("Define/Edit here instances for the current selected "
		     "printer. An instance is a combination of a real "
		     "(physical) printer and a set of predefined options. "
		     "For a single InkJet printer, you could define different "
		     "print formats like <i>DraftQuality</i>, <i>PhotoQuality</i> "
		     "or <i>TwoSided</i>. Those instances appear as normal "
		     "printers in the print dialog and allow you to quickly "
		     "select the print format you want."));
}

KMInstancePage::~KMInstancePage()
{
}

void KMInstancePage::addButton(const QString& txt, const QString& pixmap, const char *receiver)
{
	QPushButton	*btn = new QPushButton(this, 0L);
	btn->setText(txt);
	btn->setIconSet(BarIconSet(pixmap));
	btn->setFlat(true);
	connect(btn, SIGNAL(clicked()), receiver);
	m_buttons.append(btn);
}

void KMInstancePage::initActions()
{
	addButton(i18n("New..."), "filenew", SLOT(slotNew()));
	addButton(i18n("Copy..."), "editcopy", SLOT(slotCopy()));
	addButton(i18n("Remove"), "edittrash", SLOT(slotRemove()));
	m_buttons.append(0);
	addButton(i18n("Set as Default"), "exec", SLOT(slotDefault()));
	addButton(i18n("Settings..."), "configure", SLOT(slotSettings()));
	m_buttons.append(0);
	addButton(i18n("Test..."), "fileprint", SLOT(slotTest()));
}

void KMInstancePage::setPrinter(KMPrinter *p)
{
	QString	oldText = m_view->currentText();

	m_view->clear();
	m_printer = p;
	//bool	ok = (p && !p->isSpecial());
	bool	ok = (p != 0);
	if (ok)
	{
		QPtrList<KMPrinter>	list;
		KMFactory::self()->virtualManager()->virtualList(list,p->name());
		QPtrListIterator<KMPrinter>	it(list);
		for (;it.current();++it)
		{
			QStringList	pair = QStringList::split('/',it.current()->name(),false);
			m_view->insertItem(SmallIcon((it.current()->isSoftDefault() ? "exec" : "fileprint")),(pair.count() > 1 ? pair[1] : i18n("(Default)")));
		}
		m_view->sort();
	}

	for (QValueList<QButton*>::ConstIterator it=m_buttons.begin(); it!=m_buttons.end(); ++it)
		if (*it)
			(*it)->setEnabled(ok);

	//iif (!oldText.isEmpty())
	//{
		QListBoxItem	*item = m_view->findItem(oldText);
		if (!item)
			item = m_view->findItem(i18n("(Default)"));
		if (item)
			m_view->setSelected(item,true);
	//}
}

void KMInstancePage::slotNew()
{
	KMTimer::self()->hold();

	bool	ok(false);
	QString	name = KLineEditDlg::getText(i18n("Instance Name"),i18n("Enter name for new instance (leave untouched for default):"),
			                     i18n("(Default)"),&ok,this);
	if (ok)
	{
		if (name.find(QRegExp("\\s")) != -1)
			KMessageBox::error(this, i18n("Instance name may not contain any space!"));
		else
		{
			if (name == i18n("(Default)"))
				name = QString::null;
			KMFactory::self()->virtualManager()->create(m_printer,name);
			setPrinter(m_printer);
		}
	}

	KMTimer::self()->release();
}

void KMInstancePage::slotRemove()
{
	KMTimer::self()->hold();
	bool	reload(false);

	QString	src = m_view->currentText();
        QString msg = (src != i18n("(Default)") ? i18n("Do you really want to remove instance %1?") : i18n("You can't remove the default instance. However all settings of %1 will be discarded. Continue?"));
	if (!src.isEmpty() && KMessageBox::warningYesNo(this,msg.arg(src)) == KMessageBox::Yes)
	{
		if (src == i18n("(Default)"))
			src = QString::null;
		reload = KMFactory::self()->virtualManager()->isDefault(m_printer,src);
		KMFactory::self()->virtualManager()->remove(m_printer,src);
		setPrinter(m_printer);
	}

	KMTimer::self()->release(reload);
}

void KMInstancePage::slotCopy()
{
	KMTimer::self()->hold();

	QString	src = m_view->currentText();
	if (!src.isEmpty())
	{
		bool	ok(false);
		QString	name = KLineEditDlg::getText(i18n("Instance Name"),i18n("Enter name for new instance (leave untouched for default):"),
				                     i18n("(Default)"),&ok,this);
		if (ok)
		{
			if (name.find(QRegExp("\\s")) != -1)
				KMessageBox::error(this, i18n("Instance name may not contain any space!"));
			else
			{
				if (src == i18n("(Default)"))
					src = QString::null;
				if (name == i18n("(Default)"))
					name = QString::null;
				KMFactory::self()->virtualManager()->copy(m_printer,src,name);
				setPrinter(m_printer);
			}
		}
	}

	KMTimer::self()->release();
}

void KMInstancePage::slotSettings()
{
	KMTimer::self()->hold();

	QString	src = m_view->currentText();
	if (!src.isEmpty())
	{
		if (src == i18n("(Default)")) src = QString::null;
		KMPrinter	*pr = KMFactory::self()->virtualManager()->findInstance(m_printer,src);
		if (pr && (pr->isSpecial() || KMFactory::self()->manager()->completePrinterShort(pr)))
		{
			int oldAppType = KMFactory::self()->settings()->application;
			KMFactory::self()->settings()->application = -1;
			KPrinterPropertyDialog::setupPrinter(pr, this);
			KMFactory::self()->settings()->application = oldAppType;
			if (pr->isEdited())
			{ // printer edited, need to save changes
				pr->setDefaultOptions(pr->editedOptions());
				pr->setEditedOptions(QMap<QString,QString>());
				pr->setEdited(false);
				KMFactory::self()->virtualManager()->triggerSave();
			}
		}
	}

	KMTimer::self()->release();
}

void KMInstancePage::slotDefault()
{
	KMTimer::self()->hold();

	QString	src = m_view->currentText();
	if (!src.isEmpty())
	{
		if (src == i18n("(Default)"))
			src = QString::null;
		KMFactory::self()->virtualManager()->setAsDefault(m_printer,src);
		setPrinter(m_printer);
	}

	KMTimer::self()->release(true);
}

void KMInstancePage::slotTest()
{
	KMTimer::self()->hold();

	QString	src = m_view->currentText();
	if (!src.isEmpty())
	{
		if (src == i18n("(Default)"))
			src = QString::null;
		KMPrinter	*mpr = KMFactory::self()->virtualManager()->findInstance(m_printer,src);
		if (!mpr)
			KMessageBox::error(this,i18n("Internal error: printer not found."));
		else if (KMessageBox::warningContinueCancel(this, i18n("You are about to print a test page on %1. Do you want to continue?").arg(mpr->printerName()), QString::null, i18n("Print Test Page"), "printTestPage") == KMessageBox::Continue)
		{
			if (!KMFactory::self()->virtualManager()->testInstance(mpr))
				KMessageBox::error(this, i18n("Unable to send test page to %1.").arg(mpr->printerName()));
			else
				KMessageBox::information(this,i18n("Test page successfully sent to printer %1.").arg(mpr->printerName()));
		}
	}

	KMTimer::self()->release(false);
}
#include "kminstancepage.moc"

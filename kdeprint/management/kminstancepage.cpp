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
#include "kmtimer.h"
#include "kprinterpropertydialog.h"
#include "kprinter.h"

#include <qlayout.h>
#include <qinputdialog.h>
#include <kmessagebox.h>
#include <klistbox.h>
#include <kaction.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kstddirs.h>

KMInstancePage::KMInstancePage(QWidget *parent, const char *name)
: QWidget(parent,name)
{
	m_actions = new KActionCollection(this);
	m_view = new KListBox(this);
	m_toolbar = new KToolBar(this);
	m_printer = 0;

	m_toolbar->setOrientation(Qt::Vertical);
	m_toolbar->setIconText(KToolBar::IconTextRight);

	initActions();

	QHBoxLayout	*main_ = new QHBoxLayout(this, 0, 0);
	main_->addWidget(m_view);
	main_->addWidget(m_toolbar);
}

KMInstancePage::~KMInstancePage()
{
}

void KMInstancePage::initActions()
{
	KAction	*act(0);

	act = new KAction(i18n("New"),"filenew",0,this,SLOT(slotNew()),m_actions,"instance_new");
	act->plug(m_toolbar);
	act = new KAction(i18n("Copy"),"editcopy",0,this,SLOT(slotCopy()),m_actions,"instance_copy");
	act->plug(m_toolbar);
	act = new KAction(i18n("Remove"),"edittrash",0,this,SLOT(slotRemove()),m_actions,"instance_remove");
	act->plug(m_toolbar);
	m_toolbar->insertLineSeparator();
	act = new KAction(i18n("Set as default"),"exec",0,this,SLOT(slotDefault()),m_actions,"instance_default");
	act->plug(m_toolbar);
	act = new KAction(i18n("Settings"),"configure",0,this,SLOT(slotSettings()),m_actions,"instance_settings");
	act->plug(m_toolbar);
	m_toolbar->insertLineSeparator();
	act = new KAction(i18n("Test"),"fileprint",0,this,SLOT(slotTest()),m_actions,"instance_test");
	act->plug(m_toolbar);
}

void KMInstancePage::setPrinter(KMPrinter *p)
{
	QString	oldText = m_view->currentText();

	m_view->clear();
	m_printer = p;
	bool	ok = (p && !p->isSpecial());
	if (ok)
	{
		QList<KMPrinter>	list;
		KMFactory::self()->virtualManager()->virtualList(list,p->name());
		QListIterator<KMPrinter>	it(list);
		for (;it.current();++it)
		{
			QStringList	pair = QStringList::split('/',it.current()->name(),false);
			m_view->insertItem(SmallIcon((it.current()->isSoftDefault() ? "exec" : "fileprint")),(pair.count() > 1 ? pair[1] : i18n("(Default)")));
		}
		m_view->sort();
	}

	QValueList<KAction*>	acts = m_actions->actions();
	for (QValueList<KAction*>::ConstIterator it=acts.begin(); it!=acts.end(); ++it)
		(*it)->setEnabled(ok);

	if (!oldText.isEmpty())
	{
		QListBoxItem	*item = m_view->findItem(oldText);
		if (item)
			m_view->setSelected(item,true);
	}
}

void KMInstancePage::slotNew()
{
	KMTimer::blockTimer();

	bool	ok(false);
	QString	name = QInputDialog::getText(i18n("Instance name"),i18n("Enter name for new instance (leave untouched for default):"),i18n("(Default)"),&ok,this);
	if (ok)
	{
		if (name == i18n("(Default)"))
			name = QString::null;
		KMFactory::self()->virtualManager()->create(m_printer,name);
		setPrinter(m_printer);
	}

	KMTimer::releaseTimer();
}

void KMInstancePage::slotRemove()
{
	KMTimer::blockTimer();
	bool	reload(false);

	QString	src = m_view->currentText();
        QString msg = (src != i18n("(Default)") ? i18n("<nobr>Do you really want to remove instance <b>%1</b> ?</nobr>") : i18n("<nobr>You can't remove the default instance. However all<br>settings of <b>%1</b> will be discarded. Continue ?</nobr>"));
	if (!src.isEmpty() && KMessageBox::warningYesNo(this,msg.arg(src)) == KMessageBox::Yes)
	{
		if (src == i18n("(Default)"))
			src = QString::null;
		reload = KMFactory::self()->virtualManager()->isDefault(m_printer,src);
		KMFactory::self()->virtualManager()->remove(m_printer,src);
		setPrinter(m_printer);
	}

	KMTimer::releaseTimer(reload);
}

void KMInstancePage::slotCopy()
{
	KMTimer::blockTimer();

	QString	src = m_view->currentText();
	if (!src.isEmpty())
	{
		bool	ok(false);
		QString	name = QInputDialog::getText(i18n("Instance name"),i18n("Enter name for new instance (leave untouched for default):"),i18n("(Default)"),&ok,this);
		if (ok)
		{
			if (src == i18n("(Default)"))
				src = QString::null;
			if (name == i18n("(Default)"))
				name = QString::null;
			KMFactory::self()->virtualManager()->copy(m_printer,src,name);
			setPrinter(m_printer);
		}
	}

	KMTimer::releaseTimer();
}

void KMInstancePage::slotSettings()
{
	KMTimer::blockTimer();

	QString	src = m_view->currentText();
	if (!src.isEmpty())
	{
		if (src == i18n("(Default)")) src = QString::null;
		KMPrinter	*pr = KMFactory::self()->virtualManager()->findInstance(m_printer,src);
		if (pr && KMFactory::self()->manager()->completePrinterShort(pr))
		{
			KPrinter::setApplicationType(KPrinter::StandAlone);
			KPrinterPropertyDialog::setupPrinter(pr, this);
			if (pr->isEdited())
			{ // printer edited, need to save changes
				pr->setDefaultOptions(pr->editedOptions());
				pr->setEditedOptions(QMap<QString,QString>());
				pr->setEdited(false);
				KMFactory::self()->virtualManager()->triggerSave();
			}
		}
	}

	KMTimer::releaseTimer();
}

void KMInstancePage::slotDefault()
{
	KMTimer::blockTimer();

	QString	src = m_view->currentText();
	if (!src.isEmpty())
	{
		if (src == i18n("(Default)"))
			src = QString::null;
		KMFactory::self()->virtualManager()->setAsDefault(m_printer,src);
		setPrinter(m_printer);
	}

	KMTimer::releaseTimer(true);
}

void KMInstancePage::slotTest()
{
	KMTimer::blockTimer();

	QString	src = m_view->currentText();
	if (!src.isEmpty())
	{
		if (src == i18n("(Default)"))
			src = QString::null;
		KMPrinter	*mpr = KMFactory::self()->virtualManager()->findInstance(m_printer,src);
		QString		testpage = locate("data","kdeprint/testprint.ps");
		if (testpage.isEmpty())
			KMessageBox::error(this,i18n("Unable to locate test page."));
		else if (!mpr)
			KMessageBox::error(this,i18n("Internal error: printer not found."));
		else
		{
			KPrinter::setApplicationType(KPrinter::StandAlone);
			KPrinter	pr;
			pr.setPrinterName(mpr->printerName());
			pr.setSearchName(mpr->name());
			pr.setOptions(mpr->defaultOptions());
			if (!pr.printFiles(testpage))
				KMessageBox::error(this,i18n("<nobr>Unable to send test page to <b>%1</b>.</nobr>").arg(pr.printerName()));
			else
				KMessageBox::information(this,i18n("<nobr>Test page succesfully sent to printer <b>%1</b>.</nobr>").arg(pr.printerName()));
		}
	}

	KMTimer::releaseTimer(false);
}
#include "kminstancepage.moc"

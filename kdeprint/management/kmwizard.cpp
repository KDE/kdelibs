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

#include "kmwizard.h"
#include "kmwizardpage.h"
#include "kmprinter.h"
#include "kmfactory.h"
#include "kmuimanager.h"

#include <qpushbutton.h>
#include <qlabel.h>
#include <qwidgetstack.h>
#include <kmessagebox.h>
#include <qlayout.h>
#include <klocale.h>
#include <kdebug.h>
#include <kseparator.h>

#include "kmwinfopage.h"
#include "kmwpassword.h"
#include "kmwbackend.h"
#include "kmwsocket.h"
#include "kmwdriver.h"
#include "kmwdriverselect.h"
#include "kmwdrivertest.h"
#include "kmwname.h"
#include "kmwend.h"
#include "kmwclass.h"
#include "kmwlpd.h"
#include "kmwfile.h"
#include "kmwsmb.h"
#include "kmwlocal.h"

KMWizard::KMWizard(QWidget *parent, const char *name)
: QDialog(parent,name,true)
{
	m_start = KMWizard::Start;
	m_end = KMWizard::End;
	m_inclusive = true;
	m_printer = new KMPrinter();

	m_pagepool.setAutoDelete(false);

	m_stack = new QWidgetStack(this);
	m_next = new QPushButton(i18n("Next >"), this);
	m_next->setDefault(true);
	m_prev = new QPushButton(i18n("< Back"), this);
	QPushButton	*m_cancel = new QPushButton(i18n("Cancel"), this);
	m_title = new QLabel(this);
	QFont	f(m_title->font());
	f.setBold(true);
	m_title->setFont(f);
	KSeparator* sep = new KSeparator( KSeparator::HLine, this);
	sep->setFixedHeight(5);

	connect(m_cancel,SIGNAL(clicked()),SLOT(reject()));
	connect(m_next,SIGNAL(clicked()),SLOT(slotNext()));
	connect(m_prev,SIGNAL(clicked()),SLOT(slotPrev()));

	// layout
	QVBoxLayout	*main_ = new QVBoxLayout(this, 10, 0);
	QHBoxLayout	*btn_ = new QHBoxLayout(0, 0, 10);
	QHBoxLayout	*btn2_ = new QHBoxLayout(0, 0, 10);
	main_->addWidget(m_title);
	main_->addWidget(sep);
	main_->addSpacing(10);
	main_->addWidget(m_stack,1);
	main_->addSpacing(20);
	main_->addLayout(btn_);
	btn_->addStretch(1);
	btn_->addWidget(m_cancel);
	btn_->addSpacing(30);
	btn_->addStretch(1);
	btn_->addLayout(btn2_);
	btn_->addStretch(1);
	btn2_->addWidget(m_prev);
	btn2_->addWidget(m_next);

	// create standard pages
	addPage(new KMWInfoPage(this));
	m_backend = new KMWBackend(this);
	addPage(m_backend);
	addPage(new KMWPassword(this));
	addPage(new KMWSocket(this));
	addPage(new KMWDriver(this));
	addPage(new KMWDriverSelect(this));
	addPage(new KMWDriverTest(this));
	addPage(new KMWName(this));
	addPage(new KMWEnd(this));
	addPage(new KMWClass(this));
	addPage(new KMWLpd(this));
	addPage(new KMWFile(this));
	addPage(new KMWSmb(this));
	addPage(new KMWLocal(this));

	// add other pages
	KMFactory::self()->uiManager()->setupWizard(this);

	setCurrentPage(m_start,false);
	setCaption(i18n("Add printer wizard"));
	resize(400,350);
}

KMWizard::~KMWizard()
{
	delete m_printer;
}

void KMWizard::addPage(KMWizardPage *page)
{
	m_stack->addWidget(page,page->id());
	m_pagepool.insert(page->id(),page);
}

void KMWizard::setPrinter(KMPrinter *p)
{
	if (p)
	{
		m_printer->copy(*p);
		KMWizardPage	*page = (KMWizardPage*)m_stack->visibleWidget();
		if (page)
			page->initPrinter(m_printer);
	}
}

void KMWizard::configure(int start, int end, bool inclusive)
{
	m_start = start;
	m_end = end;
	m_inclusive = inclusive;
	m_pagestack.clear();

	// disable class if not starting from beginning
	m_backend->enableBackend(KMWizard::Class,(m_start == KMWizard::Start));
	setCaption((m_start == KMWizard::Start ? i18n("Add printer wizard") : i18n("Modify printer")));

	setCurrentPage(m_start,false);
}

void KMWizard::setNextPage(int page, int next)
{
	KMWizardPage	*p = m_pagepool.find(page);
	if (p)
		p->setNextPage(next);
}

void KMWizard::setCurrentPage(int ID, bool back)
{
	KMWizardPage	*page = m_pagepool.find(ID);
	if (!page)
	{
		KMessageBox::error(this,i18n("Unable to find the requested page."),i18n("Add printer wizard"));
		return;
	}

	// add current page to stack (only if going backward)
	if (!back) m_pagestack.push(ID);

	// raise page + initialize
	m_stack->raiseWidget(page);
	m_title->setText(page->title());
	if (!back) page->initPrinter(m_printer);

	// update buttons
	if (ID == m_start) m_prev->hide();
	else m_prev->show();
	if ((m_inclusive && ID == m_end) || (!m_inclusive && page->nextPage() == m_end))
		m_next->setText(i18n("OK"));
	else
		m_next->setText(i18n("Next >"));
}

void KMWizard::slotPrev()
{
	// current page is on top of stack -> pop it and retrieve new top of stack
	m_pagestack.pop();
	int	ID = m_pagestack.top();
	setCurrentPage(ID,true);
}

void KMWizard::slotNext()
{
	KMWizardPage	*page = (KMWizardPage*)m_stack->visibleWidget();
	if (page)
	{
		QString	msg;
		if (!page->isValid(msg))
		{
			if (!msg.isEmpty())
				KMessageBox::error(this,msg,page->title());
		}
		else
		{
			page->updatePrinter(m_printer);
			if (m_next->text() == i18n("OK"))
				accept();
			else
			{
				int	ID = page->nextPage();
				setCurrentPage(ID,false);
			}
		}
	}
}
#include "kmwizard.moc"

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

#include "kprintdialog.h"
#include "kprinter.h"
#include "kprinterimpl.h"
#include "kmfactory.h"
#include "kmuimanager.h"
#include "kmmanager.h"
#include "kmprinter.h"
#include "kmvirtualmanager.h"
#include "kprintdialogpage.h"
#include "kprinterpropertydialog.h"

#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <kmessagebox.h>
#include <qdir.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kfiledialog.h>

KPrintDialog::KPrintDialog(QWidget *parent, const char *name)
: KDialog(parent,name,true)
{
	m_pages.setAutoDelete(false);
	m_printer = 0;
	setCaption(i18n("Print"));

	// widget creation
	QGroupBox	*m_pbox = new QGroupBox(i18n("Printer"), this);
	m_type = new QLabel(m_pbox);
	m_state = new QLabel(m_pbox);
	m_comment = new QLabel(m_pbox);
	m_location = new QLabel(m_pbox);
	m_printers = new QComboBox(m_pbox);
	m_printers->setMinimumHeight(25);
	QLabel	*m_printerlabel = new QLabel(i18n("Name:"), m_pbox);
	QLabel	*m_statelabel = new QLabel(i18n("Status", "State:"), m_pbox);
	QLabel	*m_typelabel = new QLabel(i18n("Type:"), m_pbox);
	QLabel	*m_locationlabel = new QLabel(i18n("Location:"), m_pbox);
	QLabel	*m_commentlabel = new QLabel(i18n("Comment:"), m_pbox);
	m_properties = new QPushButton(i18n("Properties..."), m_pbox);
	m_options = new QPushButton(i18n("Options"), this);
	m_default = new QPushButton(i18n("Set as default"), m_pbox);
	m_filebrowse = new QPushButton(i18n("Browse"), m_pbox);
	m_ok = new QPushButton(i18n("OK"), this);
	m_ok->setDefault(true);
	QPushButton	*m_cancel = new QPushButton(i18n("Cancel"), this);
	m_preview = new QCheckBox(i18n("Preview"), m_pbox);
	m_printtofile = new QCheckBox(i18n("Print to file"), m_pbox);
	m_file = new QLineEdit(m_pbox);
	m_file->setEnabled(false);
	m_file->setText(QDir::homeDirPath()+"/print.ps");
	m_filebrowse->setEnabled(false);

	// layout creation
	QVBoxLayout	*l1 = new QVBoxLayout(this, 10, 10);
	l1->addWidget(m_pbox,0);
	QHBoxLayout	*l2 = new QHBoxLayout(0, 0, 10);
	l1->addLayout(l2);
	l2->addWidget(m_options,0);
	l2->addStretch(1);
	l2->addWidget(m_ok,0);
	l2->addWidget(m_cancel,0);
	QGridLayout	*l3 = new QGridLayout(m_pbox,3,2,10,10);
	l3->addRowSpacing(0,10);
	l3->setColStretch(0,1);
	l3->setRowStretch(1,1);
	QGridLayout	*l4 = new QGridLayout(0, 5, 2, 0, 5);
	l3->addLayout(l4,1,0);
	l4->addWidget(m_printerlabel,0,0);
	l4->addWidget(m_statelabel,1,0);
	l4->addWidget(m_typelabel,2,0);
	l4->addWidget(m_locationlabel,3,0);
	l4->addWidget(m_commentlabel,4,0);
	l4->addWidget(m_printers,0,1);
	l4->addWidget(m_state,1,1);
	l4->addWidget(m_type,2,1);
	l4->addWidget(m_location,3,1);
	l4->addWidget(m_comment,4,1);
	l4->setColStretch(1,1);
	QVBoxLayout	*l5 = new QVBoxLayout(0, 0, 10);
	l3->addLayout(l5,1,1);
	l5->addWidget(m_properties,0);
	l5->addWidget(m_default,0);
	l5->addWidget(m_preview,0);
	l5->addStretch(1);
	QHBoxLayout	*l6 = new QHBoxLayout(0, 0, 10);
	l3->addLayout(l6,2,0);
	l6->addWidget(m_printtofile,0);
	l6->addWidget(m_file,1);
	l3->addWidget(m_filebrowse,2,1);

	// connections
	connect(m_ok,SIGNAL(clicked()),SLOT(accept()));
	connect(m_cancel,SIGNAL(clicked()),SLOT(reject()));
	connect(m_properties,SIGNAL(clicked()),SLOT(slotProperties()));
	connect(m_default,SIGNAL(clicked()),SLOT(slotSetDefault()));
	connect(m_filebrowse,SIGNAL(clicked()),SLOT(slotBrowse()));
	connect(m_printers,SIGNAL(activated(int)),SLOT(slotPrinterSelected(int)));
	connect(m_options,SIGNAL(clicked()),SLOT(slotOptions()));
	connect(m_printtofile,SIGNAL(toggled(bool)),m_default,SLOT(setDisabled(bool)));
	connect(m_printtofile,SIGNAL(toggled(bool)),m_printers,SLOT(setDisabled(bool)));
	connect(m_printtofile,SIGNAL(toggled(bool)),m_preview,SLOT(setDisabled(bool)));
	connect(m_printtofile,SIGNAL(toggled(bool)),m_type,SLOT(setDisabled(bool)));
	connect(m_printtofile,SIGNAL(toggled(bool)),m_state,SLOT(setDisabled(bool)));
	connect(m_printtofile,SIGNAL(toggled(bool)),m_comment,SLOT(setDisabled(bool)));
	connect(m_printtofile,SIGNAL(toggled(bool)),m_location,SLOT(setDisabled(bool)));
	connect(m_printtofile,SIGNAL(toggled(bool)),m_typelabel,SLOT(setDisabled(bool)));
	connect(m_printtofile,SIGNAL(toggled(bool)),m_printerlabel,SLOT(setDisabled(bool)));
	connect(m_printtofile,SIGNAL(toggled(bool)),m_commentlabel,SLOT(setDisabled(bool)));
	connect(m_printtofile,SIGNAL(toggled(bool)),m_statelabel,SLOT(setDisabled(bool)));
	connect(m_printtofile,SIGNAL(toggled(bool)),m_locationlabel,SLOT(setDisabled(bool)));
	connect(m_printtofile,SIGNAL(toggled(bool)),m_filebrowse,SLOT(setEnabled(bool)));
	connect(m_printtofile,SIGNAL(toggled(bool)),m_file,SLOT(setEnabled(bool)));
	connect(m_printtofile,SIGNAL(toggled(bool)),this,SLOT(slotFilePrintToggled(bool)));
}

KPrintDialog::~KPrintDialog()
{
}

void KPrintDialog::setFlags(int f)
{
	if (!(f & KMUiManager::Properties)) m_properties->hide();
	if (!(f & KMUiManager::Default)) m_default->hide();
	if (!(f & KMUiManager::Preview)) m_preview->hide();
	if (!(f & KMUiManager::Options)) m_options->hide();
	if (!(f & KMUiManager::OutputToFile))
	{
		m_printtofile->hide();
		m_file->hide();
		m_filebrowse->hide();
	}
}

void KPrintDialog::setDialogPages(QList<KPrintDialogPage> *pages)
{
	if (!pages || pages->count() == 0) return;
	m_pages.clear();
	if (pages->count() == 1)
	{
		KPrintDialogPage	*page = pages->take(0);
		m_pages.append(page);
		page->reparent(this,QPoint(0,0));
		((QVBoxLayout*)layout())->insertWidget(1,page,1);
	}
	else
	{
		QTabWidget	*tabs = new QTabWidget(this);
		tabs->setMargin(10);
		((QVBoxLayout*)layout())->insertWidget(1,tabs,1);
		while (pages->count() > 0)
		{
			KPrintDialogPage	*page = pages->take(0);
			m_pages.append(page);
			tabs->addTab(page,page->title());
		}
	}
}

bool KPrintDialog::printerSetup(KPrinter *printer, QWidget *parent)
{
	if (printer)
	{
		KPrintDialog	dlg(parent);
		KMFactory::self()->uiManager()->setupPrintDialog(&dlg);
		dlg.initialize(printer);
		return dlg.exec();
	}
	return false;
}

void KPrintDialog::initialize(KPrinter *printer)
{
	m_printer = printer;

	// first retrieve printer list and update combo box (get default or last used printer also)
	QList<KMPrinter>	*plist = KMFactory::self()->manager()->printerList();
	if (plist)
	{
		m_printers->clear();
		QListIterator<KMPrinter>	it(*plist);
		int 	defindex(-1);
		for (;it.current();++it)
		{
			m_printers->insertItem(SmallIcon(it.current()->pixmap()),it.current()->name());
			if ((it.current()->isSoftDefault() && defindex == -1) || it.current()->name() == printer->searchName())
				defindex = m_printers->count()-1;
		}
		if (defindex == -1) defindex = 0;
		m_printers->setCurrentItem(defindex);
		slotPrinterSelected(defindex);
	}

	// check output to file
	if (m_printer->outputToFile())
	{
		m_printtofile->setChecked(true);
		m_file->setText(m_printer->outputFileName());
	}

	// update with KPrinter options
	if (m_printer->option("kde-preview") == "1")
		m_preview->setChecked(true);
	QListIterator<KPrintDialogPage>	it(m_pages);
	for (;it.current();++it)
		it.current()->setOptions(m_printer->options());
}

void KPrintDialog::slotPrinterSelected(int index)
{
	bool 	ok(false);
	m_location->setText(QString::null);
	m_state->setText(QString::null);
	m_comment->setText(QString::null);
	m_type->setText(QString::null);
	if (index >= 0 && index < m_printers->count())
	{
		KMManager	*mgr = KMFactory::self()->manager();
		KMPrinter	*p = mgr->findPrinter(m_printers->text(index));
		if (p)
		{
			mgr->completePrinterShort(p);
			m_location->setText(p->location());
			m_comment->setText(p->driverInfo());
			m_type->setText(p->description());
			switch (p->state())
			{
				case KMPrinter::Idle:
					m_state->setText(i18n("Printer idle"));
					break;
				case KMPrinter::Processing:
					m_state->setText(i18n("Processing..."));
					break;
				case KMPrinter::Stopped:
					m_state->setText(i18n("Stopped"));
					break;
				default:
					m_state->setText(QString::fromLatin1(""));
					break;
			}
			ok = p->isValid();
		}
	}
	m_properties->setEnabled(ok);
	m_ok->setEnabled(ok);
}

void KPrintDialog::slotBrowse()
{
	QString	fname = KFileDialog::getOpenFileName(m_file->text(),QString::null,this,i18n("Print to file"));
	if (!fname.isEmpty())
		m_file->setText(fname);
}

void KPrintDialog::slotProperties()
{
	if (!m_printer) return;

	KMPrinter	*prt(0);
	if (m_printtofile->isChecked()) prt = m_printer->implementation()->filePrinter();
	else prt = KMFactory::self()->manager()->findPrinter(m_printers->currentText());
	if (prt)
		KPrinterPropertyDialog::setupPrinter(prt, this);
}

void KPrintDialog::slotSetDefault()
{
	KMPrinter	*p = KMFactory::self()->manager()->findPrinter(m_printers->currentText());
	if (p)
		KMFactory::self()->virtualManager()->setDefault(p);
}

void KPrintDialog::done(int result)
{
	if (result == Accepted && m_printer)
	{
		QMap<QString,QString>	opts;
		KMPrinter		*prt(0);

		// get options from global pages
		QListIterator<KPrintDialogPage>	it(m_pages);
		for (;it.current();++it)
			it.current()->getOptions(opts);

		// add options from the dialog itself
		if (m_printtofile->isChecked())
		{
			if (!checkOutputFile()) return;
			m_printer->setOutputToFile(true);
			m_printer->setOutputFileName(m_file->text());
			prt = m_printer->implementation()->filePrinter();
		}
		else
		{
			// TODO: ADD PRINTER CHECK MECHANISM !!!
			prt = KMFactory::self()->manager()->findPrinter(m_printers->currentText());
			m_printer->setPrinterName(prt->printerName());
			m_printer->setSearchName(prt->name());
			m_printer->setOutputToFile(false);
		}
		opts["kde-preview"] = (m_preview->isChecked() ? "1" : "0");

		// merge options with KMPrinter object options
		QMap<QString,QString>	popts = (prt->isEdited() ? prt->editedOptions() : prt->defaultOptions());
		for (QMap<QString,QString>::ConstIterator it=popts.begin(); it!=popts.end(); ++it)
			opts[it.key()] = it.data();

		// update KPrinter object
		m_printer->setOptions(opts);

		// close dialog
		KDialog::done(result);
	}
	else
		KDialog::done(result);
}

bool KPrintDialog::checkOutputFile()
{
	bool	value(false);
	if (m_file->text().isEmpty())
		KMessageBox::error(this,i18n("The output filename is empty."));
	else
	{
		QFileInfo	f(m_file->text());
		if (f.exists())
		{
			if (f.isWritable())
				value = (KMessageBox::warningYesNo(this,i18n("File \"%1\" already exists. Overwrite ?").arg(f.absFilePath())) == KMessageBox::Yes);
			else
				KMessageBox::error(this,i18n("You don't have write permissions to this file."));
		}
		else
		{
			if (QFileInfo(f.dirPath(true)).isWritable())
				value = true;
			else
				KMessageBox::error(this,i18n("You don't have write permissions in that directory."));
		}
	}
	return value;
}

void KPrintDialog::slotFilePrintToggled(bool on)
{
	KPrintDialogPage	*copypage = (KPrintDialogPage*)child("CopiesPage","KPrintDialogPage");
	if (copypage && m_printer && m_printer->pageSelection() == KPrinter::SystemSide)
		copypage->setEnabled(!on);
	if (on)
	{
		m_ok->setEnabled(true);
		m_properties->setEnabled(true);
	}
	else
		slotPrinterSelected(m_printers->currentItem());
}

void KPrintDialog::slotOptions()
{
	if (KMFactory::self()->manager()->configure(this))
		initialize(m_printer);
}
#include "kprintdialog.moc"

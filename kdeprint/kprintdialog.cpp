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
#include "plugincombobox.h"
#include "kfilelist.h"
#include "kpcopiespage.h"

#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qtabwidget.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qregexp.h>
#include <kmessagebox.h>
#include <qdir.h>
#include <qtooltip.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kdebug.h>

#define	SHOWHIDE(widget,on)	if (on) widget->show(); else widget->hide();

KPrintDialog::KPrintDialog(QWidget *parent, const char *name)
: KDialog(parent,name,true)
{
	m_pages.setAutoDelete(false);
	m_printer = 0;
	setCaption(i18n("Print"));

	// widget creation
	QGroupBox	*m_pbox = new QGroupBox(0,Qt::Vertical,i18n("Printer"), this);
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
	m_options = new QPushButton(i18n("Options..."), this);
	m_default = new QPushButton(i18n("Set as default"), m_pbox);
	m_filebrowse = new QPushButton(i18n("Browse..."), m_pbox);
	m_wizard = new QPushButton(m_pbox);
	m_wizard->setPixmap(SmallIcon("wizard"));
	m_wizard->setMinimumSize(QSize(m_printers->minimumHeight(),m_printers->minimumHeight()));
	QToolTip::add(m_wizard, i18n("Add printer..."));
	m_ok = new QPushButton(i18n("OK"), this);
	m_ok->setDefault(true);
	QPushButton	*m_cancel = new QPushButton(i18n("Cancel"), this);
	m_preview = new QCheckBox(i18n("Preview"), m_pbox);
	m_filelabel = new QLabel(i18n("Output file:"), m_pbox);
	m_file = new QLineEdit(m_pbox);
	m_file->setEnabled(false);
	m_file->setText(QDir::homeDirPath()+"/print.ps");
	m_filebrowse->setEnabled(false);
	m_cmdlabel = new QLabel(i18n("Print command:"), m_pbox);
	m_cmd = new QLineEdit(m_pbox);
	m_dummy = new QVBox(this);
	m_plugin = new PluginComboBox(this);
	QLabel	*pluginlabel = new QLabel(i18n("Print system currently used:"), this);
	pluginlabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_fileselect = new KFileList(this);

	// layout creation
	QVBoxLayout	*l1 = new QVBoxLayout(this, 10, 10);
	l1->addWidget(m_fileselect, 0);
	l1->addWidget(m_pbox,0);
	l1->addWidget(m_dummy,1);
	QHBoxLayout	*ll1 = new QHBoxLayout(0, 0, 10);
	l1->addLayout(ll1, 0);
	ll1->addWidget(pluginlabel, 1);
	ll1->addWidget(m_plugin, 0);
	QHBoxLayout	*l2 = new QHBoxLayout(0, 0, 10);
	l1->addLayout(l2);
	l2->addWidget(m_options,0);
	l2->addStretch(1);
	l2->addWidget(m_ok,0);
	l2->addWidget(m_cancel,0);
	QGridLayout	*l3 = new QGridLayout(m_pbox->layout(),3,3,7);
	l3->setColStretch(1,1);
	l3->setRowStretch(0,1);
	QGridLayout	*l4 = new QGridLayout(0, 5, 2, 0, 5);
	l3->addMultiCellLayout(l4,0,0,0,1);
	l4->addWidget(m_printerlabel,0,0);
	l4->addWidget(m_statelabel,1,0);
	l4->addWidget(m_typelabel,2,0);
	l4->addWidget(m_locationlabel,3,0);
	l4->addWidget(m_commentlabel,4,0);
	QHBoxLayout	*ll4 = new QHBoxLayout(0, 0, 3);
	l4->addLayout(ll4,0,1);
	ll4->addWidget(m_printers,1);
	ll4->addWidget(m_wizard,0);
	//l4->addWidget(m_printers,0,1);
	l4->addWidget(m_state,1,1);
	l4->addWidget(m_type,2,1);
	l4->addWidget(m_location,3,1);
	l4->addWidget(m_comment,4,1);
	l4->setColStretch(1,1);
	QVBoxLayout	*l5 = new QVBoxLayout(0, 0, 10);
	l3->addLayout(l5,0,2);
	l5->addWidget(m_properties,0);
	l5->addWidget(m_default,0);
	l5->addWidget(m_preview,0);
	l5->addStretch(1);
	//***
	l3->addWidget(m_filelabel,1,0);
	l3->addWidget(m_file,1,1);
	l3->addWidget(m_filebrowse,1,2);
	//***
	l3->addWidget(m_cmdlabel,2,0);
	l3->addMultiCellWidget(m_cmd,2,2,1,2);

	// connections
	connect(m_ok,SIGNAL(clicked()),SLOT(accept()));
	connect(m_cancel,SIGNAL(clicked()),SLOT(reject()));
	connect(m_properties,SIGNAL(clicked()),SLOT(slotProperties()));
	connect(m_default,SIGNAL(clicked()),SLOT(slotSetDefault()));
	connect(m_filebrowse,SIGNAL(clicked()),SLOT(slotBrowse()));
	connect(m_printers,SIGNAL(activated(int)),SLOT(slotPrinterSelected(int)));
	connect(m_options,SIGNAL(clicked()),SLOT(slotOptions()));
	connect(m_wizard,SIGNAL(clicked()),SLOT(slotWizard()));
}

KPrintDialog::~KPrintDialog()
{
}

void KPrintDialog::setFlags(int f)
{
	SHOWHIDE(m_fileselect, (f & KMUiManager::FileSelect));
	SHOWHIDE(m_properties, (f & KMUiManager::Properties))
	SHOWHIDE(m_default, (f & KMUiManager::Default))
	SHOWHIDE(m_preview, (f & KMUiManager::Preview))
	SHOWHIDE(m_options, (f & KMUiManager::Options))
	bool	on = (f & KMUiManager::OutputToFile);
	SHOWHIDE(m_filelabel, on)
	SHOWHIDE(m_file, on)
	SHOWHIDE(m_filebrowse, on)
	on = (f & KMUiManager::PrintCommand);
	SHOWHIDE(m_cmdlabel, on)
	SHOWHIDE(m_cmd, on)

	// also update "wizard" button
	KMManager	*mgr = KMFactory::self()->manager();
	m_wizard->setEnabled((mgr->hasManagement() && (mgr->printerOperationMask() & KMManager::PrinterCreation)));
}

void KPrintDialog::setDialogPages(QPtrList<KPrintDialogPage> *pages)
{
	if (!pages) return;
	if (pages->count() + m_pages.count() == 1)
	{
		// only one page, reparent the page to m_dummy and remove any
		// QTabWidget child if any.
		if (pages->count() > 0)
			m_pages.append(pages->take(0));
		m_pages.first()->reparent(m_dummy, QPoint(0,0));
		m_pages.first()->show();
		delete m_dummy->child("TabWidget", "QTabWidget");
	}
	else
	{
		// more than one page.
		QTabWidget	*tabs = static_cast<QTabWidget*>(m_dummy->child("TabWidget", "QTabWidget"));
		if (!tabs)
		{
			// QTabWidget doesn't exist. Create it and reparent all
			// already existing pages.
			tabs = new QTabWidget(m_dummy, "TabWidget");
			tabs->setMargin(10);
			for (m_pages.first(); m_pages.current(); m_pages.next())
			{
				tabs->addTab(m_pages.current(), m_pages.current()->title());
			}
		}
		while (pages->count() > 0)
		{
			KPrintDialogPage	*page = pages->take(0);
			m_pages.append(page);
			tabs->addTab(page, page->title());
		}
		tabs->show();
	}
}

bool KPrintDialog::printerSetup(KPrinter *printer, QWidget *parent, const QString& caption)
{
	if (printer)
	{
		KPrintDialog	dlg(parent);
		KMFactory::self()->uiManager()->setupPrintDialog(&dlg);
		dlg.initialize(printer);
		if (!caption.isEmpty())
			dlg.setCaption(caption);
		return dlg.exec();
	}
	return false;
}

void KPrintDialog::initialize(KPrinter *printer)
{
	m_printer = printer;

	// first retrieve printer list and update combo box (get default or last used printer also)
	QPtrList<KMPrinter>	*plist = KMFactory::self()->manager()->printerList();
	if (plist)
	{
		m_printers->clear();
		QPtrListIterator<KMPrinter>	it(*plist);
		int 	defsoft(-1), defhard(-1), defsearch(-1);
		bool	sep(false);
		for (;it.current();++it)
		{
			if (!sep && it.current()->isSpecial())
			{
				sep = true;
				m_printers->insertItem(QString::fromLatin1("--------"));
			}
			m_printers->insertItem(SmallIcon(it.current()->pixmap(),0,(it.current()->isValid() ? (int)KIcon::DefaultState : (int)KIcon::LockOverlay)),it.current()->name());
			if (it.current()->isSoftDefault())
				defsoft = m_printers->count()-1;
			if (it.current()->isHardDefault())
				defhard = m_printers->count()-1;
			if (it.current()->name() == printer->searchName())
				defsearch = m_printers->count()-1;
		}
		int	defindex = (defsearch != -1 ? defsearch : (defsoft != -1 ? defsoft : QMAX(defhard,0)));
		m_printers->setCurrentItem(defindex);
		slotPrinterSelected(defindex);
	}

	// Initialize output filename
	if (!m_printer->outputFileName().isEmpty())
		m_file->setText(m_printer->outputFileName());

	// update with KPrinter options
	if (m_printer->option("kde-preview") == "1" || m_printer->previewOnly())
		m_preview->setChecked(true);
	m_preview->setEnabled(!m_printer->previewOnly());
	m_cmd->setText(m_printer->option("kde-printcommand"));
	m_fileselect->setFileList(QStringList::split(QRegExp(",\\s*"), m_printer->option("kde-filelist"), false));
	QPtrListIterator<KPrintDialogPage>	it(m_pages);
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
			if (!p->isSpecial()) mgr->completePrinterShort(p);
			m_location->setText(p->location());
			m_comment->setText(p->driverInfo());
			m_type->setText(p->description());
			m_state->setText(p->stateString());
			ok = p->isValid();
			enableSpecial(p->isSpecial());
			enableOutputFile(p->option("kde-special-file") == "1");
			setOutputFileExtension(p->option("kde-special-extension"));
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

	KMPrinter	*prt = KMFactory::self()->manager()->findPrinter(m_printers->currentText());
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
		QString	msg;
		QPtrListIterator<KPrintDialogPage>	it(m_pages);
		for (;it.current();++it)
			if (it.current()->isEnabled())
			{
				if (it.current()->isValid(msg))
					it.current()->getOptions(opts);
				else
				{
					KMessageBox::error(this, msg);
					return;
				}
			}

		// add options from the dialog itself
		// TODO: ADD PRINTER CHECK MECHANISM !!!
		prt = KMFactory::self()->manager()->findPrinter(m_printers->currentText());
		if (prt->isSpecial() && prt->option("kde-special-file") == "1")
		{
			if (!checkOutputFile()) return;
			m_printer->setOutputToFile(true);
			m_printer->setOutputFileName(m_file->text());
		}
		else
			m_printer->setOutputToFile(false);
		m_printer->setPrinterName(prt->printerName());
		m_printer->setSearchName(prt->name());
		opts["kde-printcommand"] = m_cmd->text();
		opts["kde-preview"] = (m_preview->isChecked() ? "1" : "0");
		opts["kde-isspecial"] = (prt->isSpecial() ? "1" : "0");
		opts["kde-special-command"] = prt->option("kde-special-command");
		if (m_fileselect->isVisible())
			opts["kde-filelist"] = m_fileselect->fileList().join(", ");

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

void KPrintDialog::slotOptions()
{
	if (KMFactory::self()->manager()->configure(this))
		initialize(m_printer);
}

void KPrintDialog::enableOutputFile(bool on)
{
	m_filelabel->setEnabled(on);
	m_file->setEnabled(on);
	m_filebrowse->setEnabled(on);
}

void KPrintDialog::enableSpecial(bool on)
{
	m_default->setDisabled(on);
	m_cmdlabel->setDisabled(on);
	m_cmd->setDisabled(on);
	KPCopiesPage	*copypage = (KPCopiesPage*)child("CopiesPage","KPCopiesPage");
	if (copypage)
		copypage->initialize(!on);
	// disable/enable all other pages (if needed)
	for (m_pages.first(); m_pages.current(); m_pages.next())
		if (m_pages.current()->onlyRealPrinters())
			m_pages.current()->setEnabled(!on);
}

void KPrintDialog::setOutputFileExtension(const QString& ext)
{
	if (!ext.isEmpty())
	{
		QFileInfo	fi(m_file->text());
		QString		str = fi.dirPath(true)+"/"+fi.baseName()+"."+ext;
		m_file->setText(QDir::cleanDirPath(str));
	}
}

void KPrintDialog::slotWizard()
{
	int	result = KMFactory::self()->manager()->addPrinterWizard(this);
	if (result == -1)
		KMessageBox::error(this, KMFactory::self()->manager()->errorMsg());
	else if (result == 1)
		initialize(m_printer);
}

void KPrintDialog::reload()
{
	// remove printer dependent pages (usually from plugin)
	QTabWidget	*tabs = static_cast<QTabWidget*>(m_dummy->child("TabWidget", "QTabWidget"));
	for (int i=0; i<m_pages.count(); i++)
		if (m_pages.at(i)->onlyRealPrinters())
		{
			KPrintDialogPage	*page = m_pages.take(i--);
			if (tabs)
				tabs->removePage(page);
			delete page;
		}
	// reload printer dependent pages from plugin
	QPtrList<KPrintDialogPage>	pages;
	pages.setAutoDelete(false);
	KMFactory::self()->uiManager()->setupPrintDialogPages(&pages);
	// add those pages to the dialog
	setDialogPages(&pages);
	m_dummy->show();
	// other initializations
	setFlags(KMFactory::self()->uiManager()->dialogFlags());
	initialize(m_printer);
}

#include "kprintdialog.moc"

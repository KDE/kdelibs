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
#include "kpcopiespage.h"
#include "treecombobox.h"
#include "messagewindow.h"

#include <qgroupbox.h>
#include <qcheckbox.h>
#include <kpushbutton.h>
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
#include <kurlrequester.h>
#include <klineedit.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kguiitem.h>
#include <kstdguiitem.h>
#include <kapplication.h>

#define	SHOWHIDE(widget,on)	if (on) widget->show(); else widget->hide();

class KPrintDialog::KPrintDialogPrivate
{
public:
	QLabel	*m_type, *m_state, *m_comment, *m_location, *m_cmdlabel, *m_filelabel;
	QPushButton	*m_properties, *m_default, *m_options, *m_ok, *m_wizard, *m_extbtn, *m_filter;
	QCheckBox	*m_preview;
	QLineEdit	*m_cmd;
	TreeComboBox	*m_printers;
	QVBox		*m_dummy;
	PluginComboBox	*m_plugin;
	KURLRequester	*m_file;
	QCheckBox	*m_persistent;
	bool	m_reduced;
	MessageWindow *m_msgwindow;

	QPtrList<KPrintDialogPage>	m_pages;
	KPrinter		*m_printer;
};

KPrintDialog::KPrintDialog(QWidget *parent, const char *name)
: KDialog(parent,name,true)
{
	d = new KPrintDialogPrivate;

	d->m_pages.setAutoDelete(false);
	d->m_printer = 0;
	d->m_msgwindow = 0;
	setCaption(i18n("Print"));

	// widget creation
	QGroupBox	*m_pbox = new QGroupBox(0,Qt::Vertical,i18n("Printer"), this);
	d->m_type = new QLabel(m_pbox);
	d->m_state = new QLabel(m_pbox);
	d->m_comment = new QLabel(m_pbox);
	d->m_location = new QLabel(m_pbox);
	d->m_printers = new TreeComboBox(m_pbox);
	d->m_printers->setMinimumHeight(25);
	QLabel	*m_printerlabel = new QLabel(i18n("&Name:"), m_pbox);
	QLabel	*m_statelabel = new QLabel(i18n("Status", "State:"), m_pbox);
	QLabel	*m_typelabel = new QLabel(i18n("Type:"), m_pbox);
	QLabel	*m_locationlabel = new QLabel(i18n("Location:"), m_pbox);
	QLabel	*m_commentlabel = new QLabel(i18n("Comment:"), m_pbox);
	m_printerlabel->setBuddy(d->m_printers);
	d->m_properties = new KPushButton(KGuiItem(i18n("P&roperties..."), "edit"), m_pbox);
	d->m_options = new KPushButton(KGuiItem(i18n("System Op&tions..."), "kdeprint_configmgr"), this);
	d->m_default = new KPushButton(KGuiItem(i18n("Set as &Default"), "kdeprint_defaultsoft"), m_pbox);
	d->m_filter = new QPushButton(m_pbox);
	d->m_filter->setPixmap(SmallIcon("filter"));
	d->m_filter->setMinimumSize(QSize(d->m_printers->minimumHeight(),d->m_printers->minimumHeight()));
	d->m_filter->setToggleButton(true);
	d->m_filter->setOn(KMManager::self()->isFilterEnabled());
	QToolTip::add(d->m_filter, i18n("Toggle Printer Filtering"));
	d->m_wizard = new QPushButton(m_pbox);
	d->m_wizard->setPixmap(SmallIcon("wizard"));
	d->m_wizard->setMinimumSize(QSize(d->m_printers->minimumHeight(),d->m_printers->minimumHeight()));
	QToolTip::add(d->m_wizard, i18n("Add printer..."));
	d->m_ok = new KPushButton(KGuiItem(i18n("&Print"), "fileprint"), this);
	d->m_ok->setDefault(true);
	d->m_ok->setEnabled( false );
	QPushButton	*m_cancel = new KPushButton(KStdGuiItem::cancel(), this);
	d->m_preview = new QCheckBox(i18n("Previe&w"), m_pbox);
	d->m_filelabel = new QLabel(i18n("O&utput file:"), m_pbox);
	d->m_file = new KURLRequester(QDir::homeDirPath()+"/print.ps", m_pbox);
	d->m_file->fileDialog()->setCaption(i18n("Print to file"));
	d->m_file->setEnabled(false);
	d->m_filelabel->setBuddy(d->m_file);
	d->m_cmdlabel = new QLabel(i18n("Print co&mmand:"), m_pbox);
	d->m_cmd = new QLineEdit(m_pbox);
	d->m_cmdlabel->setBuddy(d->m_cmd);
	d->m_dummy = new QVBox(this);
	d->m_plugin = new PluginComboBox(this);
	d->m_extbtn = new KPushButton(this);
	QToolTip::add(d->m_extbtn, i18n("Show/Hide Advanced Options"));
	d->m_persistent = new QCheckBox(i18n("&Keep this dialog open after printing"), this);
	QPushButton	*m_help = new KPushButton(KGuiItem(i18n("&Help"), "help"), this);

	// layout creation
	QVBoxLayout	*l1 = new QVBoxLayout(this, 10, 10);
	l1->addWidget(m_pbox,0);
	l1->addWidget(d->m_dummy,1);
	l1->addWidget(d->m_plugin,0);
	l1->addWidget(d->m_persistent);
	QHBoxLayout	*l2 = new QHBoxLayout(0, 0, 10);
	l1->addLayout(l2);
	l2->addWidget(d->m_extbtn,0);
	l2->addWidget(d->m_options,0);
	l2->addWidget(m_help,0);
	l2->addStretch(1);
	l2->addWidget(d->m_ok,0);
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
	ll4->addWidget(d->m_printers,1);
	ll4->addWidget(d->m_filter,0);
	ll4->addWidget(d->m_wizard,0);
	//l4->addWidget(d->m_printers,0,1);
	l4->addWidget(d->m_state,1,1);
	l4->addWidget(d->m_type,2,1);
	l4->addWidget(d->m_location,3,1);
	l4->addWidget(d->m_comment,4,1);
	l4->setColStretch(1,1);
	QVBoxLayout	*l5 = new QVBoxLayout(0, 0, 10);
	l3->addLayout(l5,0,2);
	l5->addWidget(d->m_properties,0);
	l5->addWidget(d->m_default,0);
	l5->addWidget(d->m_preview,0);
	l5->addStretch(1);
	//***
	l3->addWidget(d->m_filelabel,1,0);
	l3->addWidget(d->m_file,1,1);
	//***
	l3->addWidget(d->m_cmdlabel,2,0);
	l3->addMultiCellWidget(d->m_cmd,2,2,1,2);

	// connections
	connect(d->m_ok,SIGNAL(clicked()),SLOT(accept()));
	connect(m_cancel,SIGNAL(clicked()),SLOT(reject()));
	connect(d->m_properties,SIGNAL(clicked()),SLOT(slotProperties()));
	connect(d->m_default,SIGNAL(clicked()),SLOT(slotSetDefault()));
	connect(d->m_printers,SIGNAL(activated(int)),SLOT(slotPrinterSelected(int)));
	connect(d->m_options,SIGNAL(clicked()),SLOT(slotOptions()));
	connect(d->m_wizard,SIGNAL(clicked()),SLOT(slotWizard()));
	connect(d->m_extbtn, SIGNAL(clicked()), SLOT(slotExtensionClicked()));
	connect(d->m_filter, SIGNAL(toggled(bool)), SLOT(slotToggleFilter(bool)));
	connect(m_help, SIGNAL(clicked()), SLOT(slotHelp()));
	connect(d->m_file, SIGNAL(urlSelected(const QString&)), SLOT(slotOutputFileSelected(const QString&)));
	connect( KMFactory::self()->manager(), SIGNAL( updatePossible( bool ) ), SLOT( slotUpdatePossible( bool ) ) );

	KConfig	*config = KGlobal::config();
	config->setGroup("KPrinter Settings");
	expandDialog(!config->readBoolEntry("DialogReduced", (KMFactory::self()->settings()->application != KPrinter::StandAlone)));
}

KPrintDialog::~KPrintDialog()
{
	KConfig	*config = KGlobal::config();
	config->setGroup("KPrinter Settings");
	config->writeEntry("DialogReduced", d->m_reduced);

	delete d;
}

void KPrintDialog::setFlags(int f)
{
	SHOWHIDE(d->m_properties, (f & KMUiManager::Properties))
	d->m_default->hide();
	SHOWHIDE(d->m_default, ((f & KMUiManager::Default) && !KMFactory::self()->printConfig("General")->readBoolEntry("UseLast", true)))
	SHOWHIDE(d->m_preview, (f & KMUiManager::Preview))
	bool	on = (f & KMUiManager::OutputToFile);
	SHOWHIDE(d->m_filelabel, on)
	SHOWHIDE(d->m_file, on)
	on = (f & KMUiManager::PrintCommand);
	SHOWHIDE(d->m_cmdlabel, on)
	SHOWHIDE(d->m_cmd, on)
	SHOWHIDE(d->m_persistent, (f & KMUiManager::Persistent))

	// also update "wizard" button
	KMManager	*mgr = KMFactory::self()->manager();
	d->m_wizard->setEnabled((mgr->hasManagement() && (mgr->printerOperationMask() & KMManager::PrinterCreation)));
}

void KPrintDialog::setDialogPages(QPtrList<KPrintDialogPage> *pages)
{
	if (!pages) return;
	if (pages->count() + d->m_pages.count() == 1)
	{
		// only one page, reparent the page to d->m_dummy and remove any
		// QTabWidget child if any.
		if (pages->count() > 0)
			d->m_pages.append(pages->take(0));
		d->m_pages.first()->reparent(d->m_dummy, QPoint(0,0));
		d->m_pages.first()->show();
		delete d->m_dummy->child("TabWidget", "QTabWidget");
	}
	else
	{
		// more than one page.
		QTabWidget	*tabs = static_cast<QTabWidget*>(d->m_dummy->child("TabWidget", "QTabWidget"));
		if (!tabs)
		{
			// QTabWidget doesn't exist. Create it and reparent all
			// already existing pages.
			tabs = new QTabWidget(d->m_dummy, "TabWidget");
			tabs->setMargin(10);
			for (d->m_pages.first(); d->m_pages.current(); d->m_pages.next())
			{
				tabs->addTab(d->m_pages.current(), d->m_pages.current()->title());
			}
		}
		while (pages->count() > 0)
		{
			KPrintDialogPage	*page = pages->take(0);
			d->m_pages.append(page);
			tabs->addTab(page, page->title());
		}
		tabs->show();
	}
	d->m_extbtn->setEnabled(d->m_pages.count() > 0);
}

KPrintDialog* KPrintDialog::printerDialog(KPrinter *printer, QWidget *parent, const QString& caption, bool forceExpand)
{
	if (printer)
	{
		KPrintDialog	*dlg = new KPrintDialog(parent);
		// needs to set the printer before setting up the
		// print dialog as some additional pages may need it.
		// Real initialization comes after.
		dlg->d->m_printer = printer;
		KMFactory::self()->uiManager()->setupPrintDialog(dlg);
		dlg->init();
		if (!caption.isEmpty())
			dlg->setCaption(caption);
		if (forceExpand)
		{
			// we force the dialog to be expanded:
			//	- expand the dialog
			//	- hide the show/hide button
			dlg->expandDialog(true);
			dlg->d->m_extbtn->hide();
		}
		return dlg;
	}
	return NULL;
}

void KPrintDialog::initialize(KPrinter *printer)
{
	d->m_printer = printer;

	// first retrieve printer list and update combo box (get default or last used printer also)
	QPtrList<KMPrinter>	*plist = KMFactory::self()->manager()->printerList();
	if (!KMManager::self()->errorMsg().isEmpty())
	{
		KMessageBox::error(parentWidget(),
			"<qt><nobr>"+
			i18n("An error occurred while retrieving the printer list:")
			+"</nobr><br><br>"+KMManager::self()->errorMsg()+"</qt>");
	}

	if (plist)
	{
		QString	oldP = d->m_printers->currentText();
		d->m_printers->clear();
		QPtrListIterator<KMPrinter>	it(*plist);
		int 	defsoft(-1), defhard(-1), defsearch(-1);
		bool	sep(false);
		for (;it.current();++it)
		{
			// skip invalid printers
			if ( !it.current()->isValid() )
				continue;

			if (!sep && it.current()->isSpecial())
			{
				sep = true;
				d->m_printers->insertItem(QPixmap(), QString::fromLatin1("--------"));
			}
			d->m_printers->insertItem(SmallIcon(it.current()->pixmap(),0,(it.current()->isValid() ? (int)KIcon::DefaultState : (int)KIcon::LockOverlay)),it.current()->name(),false/*sep*/);
			if (it.current()->isSoftDefault())
				defsoft = d->m_printers->count()-1;
			if (it.current()->isHardDefault())
				defhard = d->m_printers->count()-1;
			if (!oldP.isEmpty() && oldP == it.current()->name())
				defsearch = d->m_printers->count()-1;
			else if (defsearch == -1 && it.current()->name() == printer->searchName())
				defsearch = d->m_printers->count()-1;
		}
		int	defindex = (defsearch != -1 ? defsearch : (defsoft != -1 ? defsoft : QMAX(defhard,0)));
		d->m_printers->setCurrentItem(defindex);
		slotPrinterSelected(defindex);
	}

	// Initialize output filename
	if (!d->m_printer->outputFileName().isEmpty())
		d->m_file->lineEdit()->setText(d->m_printer->outputFileName());
	else if (!d->m_printer->docFileName().isEmpty())
		d->m_file->lineEdit()->setText(QDir::homeDirPath()+"/"+d->m_printer->docFileName()+".ps");

	// update with KPrinter options
	if (d->m_printer->option("kde-preview") == "1" || d->m_printer->previewOnly())
		d->m_preview->setChecked(true);
	d->m_preview->setEnabled(!d->m_printer->previewOnly());
	d->m_cmd->setText(d->m_printer->option("kde-printcommand"));
	QPtrListIterator<KPrintDialogPage>	it(d->m_pages);
	for (;it.current();++it)
		it.current()->setOptions(d->m_printer->options());
}

void KPrintDialog::slotPrinterSelected(int index)
{
	bool 	ok(false);
	d->m_location->setText(QString::null);
	d->m_state->setText(QString::null);
	d->m_comment->setText(QString::null);
	d->m_type->setText(QString::null);
	if (index >= 0 && index < d->m_printers->count())
	{
		KMManager	*mgr = KMFactory::self()->manager();
		KMPrinter	*p = mgr->findPrinter(d->m_printers->text(index));
		if (p)
		{
			if (!p->isSpecial()) mgr->completePrinterShort(p);
			d->m_location->setText(p->location());
			d->m_comment->setText(p->driverInfo());
			d->m_type->setText(p->description());
			d->m_state->setText(p->stateString());
			ok = p->isValid();
			enableSpecial(p->isSpecial());
			enableOutputFile(p->option("kde-special-file") == "1");
			setOutputFileExtension(p->option("kde-special-extension"));
		}
	}
	d->m_properties->setEnabled(ok);
	d->m_ok->setEnabled(ok);
}

void KPrintDialog::slotProperties()
{
	if (!d->m_printer) return;

	KMPrinter	*prt = KMFactory::self()->manager()->findPrinter(d->m_printers->currentText());
	if (prt)
		KPrinterPropertyDialog::setupPrinter(prt, this);
}

void KPrintDialog::slotSetDefault()
{
	KMPrinter	*p = KMFactory::self()->manager()->findPrinter(d->m_printers->currentText());
	if (p)
		KMFactory::self()->virtualManager()->setDefault(p);
}

void KPrintDialog::done(int result)
{
	if (result == Accepted && d->m_printer)
	{
		QMap<QString,QString>	opts;
		KMPrinter		*prt(0);

		// get options from global pages
		QString	msg;
		QPtrListIterator<KPrintDialogPage>	it(d->m_pages);
		for (;it.current();++it)
			if (it.current()->isEnabled())
			{
				if (it.current()->isValid(msg))
					it.current()->getOptions(opts);
				else
				{
					KMessageBox::error(this, msg.prepend("<qt>").append("</qt>"));
					return;
				}
			}

		// add options from the dialog itself
		// TODO: ADD PRINTER CHECK MECHANISM !!!
		prt = KMFactory::self()->manager()->findPrinter(d->m_printers->currentText());
		if (prt->isSpecial() && prt->option("kde-special-file") == "1")
		{
			if (!checkOutputFile()) return;
			d->m_printer->setOutputToFile(true);
			d->m_printer->setOutputFileName(d->m_file->lineEdit()->text());
		}
		else
			d->m_printer->setOutputToFile(false);
		d->m_printer->setPrinterName(prt->printerName());
		d->m_printer->setSearchName(prt->name());
		opts["kde-printcommand"] = d->m_cmd->text();
		opts["kde-preview"] = (d->m_preview->isChecked() ? "1" : "0");
		opts["kde-isspecial"] = (prt->isSpecial() ? "1" : "0");
		opts["kde-special-command"] = prt->option("kde-special-command");
		
		// merge options with KMPrinter object options
		QMap<QString,QString>	popts = (prt->isEdited() ? prt->editedOptions() : prt->defaultOptions());
		for (QMap<QString,QString>::ConstIterator it=popts.begin(); it!=popts.end(); ++it)
			opts[it.key()] = it.data();

		// update KPrinter object
		d->m_printer->setOptions(opts);

		emit printRequested(d->m_printer);
		// close dialog if not persistent
		if (!d->m_persistent->isChecked() || !d->m_persistent->isVisible())
			KDialog::done(result);
	}
	else
		KDialog::done(result);
}

bool KPrintDialog::checkOutputFile()
{
	bool	value(false);
	if (d->m_file->lineEdit()->text().isEmpty())
		KMessageBox::error(this,i18n("The output filename is empty."));
	else
	{
		QFileInfo	f(d->m_file->lineEdit()->text());
		if (f.exists())
		{
			if (f.isWritable())
				value = (KMessageBox::warningYesNo(this,i18n("File \"%1\" already exists. Overwrite?").arg(f.absFilePath())) == KMessageBox::Yes);
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
	if (KMManager::self()->invokeOptionsDialog(this))
		init();
}

void KPrintDialog::enableOutputFile(bool on)
{
	d->m_filelabel->setEnabled(on);
	d->m_file->setEnabled(on);
}

void KPrintDialog::enableSpecial(bool on)
{
	d->m_default->setDisabled(on);
	d->m_cmdlabel->setDisabled(on);
	d->m_cmd->setDisabled(on);
	KPCopiesPage	*copypage = (KPCopiesPage*)child("CopiesPage","KPCopiesPage");
	if (copypage)
		copypage->initialize(!on);
	// disable/enable all other pages (if needed)
	for (d->m_pages.first(); d->m_pages.current(); d->m_pages.next())
		if (d->m_pages.current()->onlyRealPrinters())
			d->m_pages.current()->setEnabled(!on);
}

void KPrintDialog::setOutputFileExtension(const QString& ext)
{
	if (!ext.isEmpty())
	{
		QFileInfo	fi(d->m_file->lineEdit()->text());
		QString		str = fi.dirPath(true)+"/"+fi.baseName()+"."+ext;
		d->m_file->lineEdit()->setText(QDir::cleanDirPath(str));
	}
}

void KPrintDialog::slotWizard()
{
	int	result = KMFactory::self()->manager()->addPrinterWizard(this);
	if (result == -1)
		KMessageBox::error(this, KMFactory::self()->manager()->errorMsg().prepend("<qt>").append("</qt>"));
	else if (result == 1)
		initialize(d->m_printer);
}

void KPrintDialog::reload()
{
	// remove printer dependent pages (usually from plugin)
	QTabWidget	*tabs = static_cast<QTabWidget*>(d->m_dummy->child("TabWidget", "QTabWidget"));
	for (uint i=0; i<d->m_pages.count(); i++)
		if (d->m_pages.at(i)->onlyRealPrinters())
		{
			KPrintDialogPage	*page = d->m_pages.take(i--);
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
	if (!d->m_reduced)
		d->m_dummy->show();
	// other initializations
	setFlags(KMFactory::self()->uiManager()->dialogFlags());
	connect( KMFactory::self()->manager(), SIGNAL( updatePossible( bool ) ), SLOT( slotUpdatePossible( bool ) ) );
	init();
}

void KPrintDialog::configChanged()
{
	// simply update the printer list: do it all the time
	// as changing settings may influence the way printer
	// are listed.
	init();

	// update the GUI
	setFlags(KMFactory::self()->uiManager()->dialogFlags());
}

void KPrintDialog::expandDialog(bool on)
{
	QSize	sz(size());
	bool	needResize(isVisible());

	if (on)
	{
		sz.setHeight(sz.height()+d->m_dummy->minimumSize().height()+layout()->spacing());
		if (isVisible() || !d->m_dummy->isVisible())
			d->m_dummy->show();
		d->m_extbtn->setIconSet(SmallIconSet("up"));
		d->m_extbtn->setText(i18n("Collaps&e"));
		d->m_reduced = false;
	}
	else
	{
		sz.setHeight(sz.height()-d->m_dummy->height()-layout()->spacing());
		if (!isVisible() || d->m_dummy->isVisible())
			d->m_dummy->hide();
		d->m_extbtn->setIconSet(SmallIconSet("down"));
		d->m_extbtn->setText(i18n("&Expand"));
		d->m_reduced = true;
	}

	if (needResize)
	{
		layout()->activate();
		resize(sz);
	}
}

void KPrintDialog::slotExtensionClicked()
{
	// As all pages are children of d->m_dummy, I simply have to hide/show it
	expandDialog(!(d->m_dummy->isVisible()));
}

KPrinter* KPrintDialog::printer() const
{
	return d->m_printer;
}

void KPrintDialog::slotToggleFilter(bool on)
{
	KMManager::self()->enableFilter(on);
	initialize(d->m_printer);
}

void KPrintDialog::slotHelp()
{
	kapp->invokeHelp(QString::null, "kdeprint");
}

void KPrintDialog::slotOutputFileSelected(const QString& txt)
{
	KMPrinter	*prt = KMFactory::self()->manager()->findPrinter(d->m_printers->currentText());
	if (prt && prt->isSpecial())
	{
		QString	ext("."+prt->option("kde-special-extension"));
		if (ext.length() > 1 && txt.right(ext.length()) != ext)
		{
			d->m_file->lineEdit()->setText(txt+ext);
		}
	}
}

void KPrintDialog::init()
{
	d->m_ok->setEnabled( false );
	delete d->m_msgwindow;
	d->m_msgwindow = new MessageWindow( i18n( "Initializing printing system..." ), 500, this, "MessageWindow" );
	KMFactory::self()->manager()->checkUpdatePossible();
}

void KPrintDialog::slotUpdatePossible( bool flag )
{
	delete d->m_msgwindow;
	d->m_msgwindow = 0;
	if ( !flag )
		KMessageBox::error(parentWidget(),
			"<qt><nobr>"+
			i18n("An error occurred while retrieving the printer list:")
			+"</nobr><br><br>"+KMManager::self()->errorMsg()+"</qt>");
	initialize( d->m_printer );
}

#include "kprintdialog.moc"

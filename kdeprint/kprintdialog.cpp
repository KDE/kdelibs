#include "kprintdialog.h"

#include "kprinter.h"
#include "kdialogimpl.h"
#include "kprintdialogpage.h"
#include "kprinterpropertydialog.h"

#include <qtabwidget.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qfileinfo.h>
#include <klocale.h>
#include <kfiledialog.h>

#include <stdlib.h>

KPrintDialog::KPrintDialog(KPrinter *printer, QWidget *parent, const char *name)
: KPrintDialogBase(parent,name,true), printer_(printer)
{
	printerslist_ = 0;
	if (printer_)
		initialize();
	pages_.setAutoDelete(false);
	currentpage_ = 0;
}

KPrintDialog::~KPrintDialog()
{
}

void KPrintDialog::reloadPrinters()
{
	// save current selected printer (if one)
	int	current_ = (dest_->count() > 0 ? dest_->currentItem() : -1);

	// retrieve printers list from implementation
	printerslist_ = printer_->implementation()->printersList();
	KPrinterItemListIterator	it(*printerslist_);

	dest_->clear();
	for (;it.current();++it)
	{
		dest_->insertItem(it.current()->pixmap(),it.current()->displayName());
		if (it.current()->isDefault()) dest_->setCurrentItem(dest_->count()-1);
	}

	// set the saved current printer
	if (current_ >= 0 && current_ < (int)(dest_->count()))
		dest_->setCurrentItem(current_);

	// check if OK
	printerSelected(dest_->currentItem());
	//setValidPrinter(dest_->count() > 0);
}

void KPrintDialog::initialize()
{
	// default output filename
	outputfilename_->setText(QString("%1/print.ps").arg(QString::fromLatin1(getenv("HOME"))));

	reloadPrinters();

	// add pages
	pages_.clear();
	QList<KPrintDialogPage>	*pages = printer_->implementation()->dialogPages();
	if (pages->count() > 1)
	{
		// create tab-widget
		tabs_ = new QTabWidget(this);
		tabs_->setMargin(6);
		KPrintDialogBaseLayout->insertWidget(1,tabs_);
		connect(tabs_,SIGNAL(currentChanged(QWidget*)),SLOT(pageSelected(QWidget*)));

		while (pages->count() > 0)
		{
			KPrintDialogPage	*page = pages->take(0);
			tabs_->addTab(page,page->pageTitle());
			pages_.append(page);
		}
	}
	else if (pages->count() > 0)
	{ // do not create a tab widget if only 1 page
		KPrintDialogPage	*page = pages->take(0);
		page->reparent(this, QPoint(0,0));
		pages_.append(page);
		KPrintDialogBaseLayout->insertWidget(1,page);
	}

	// hide some widgets if necessary
	int	f = printer_->implementation()->dialogFlags();
	if (!(f & KPrinter::Default)) defaultbutton_->hide();
	if (!(f & KPrinter::Options)) optionsbutton_->hide();
	if (!(f & KPrinter::OutputToFile))
	{
		outputtofile_->hide();
		outputfilename_->hide();
		browsebutton_->hide();
	}
	if (!(f & KPrinter::Preview)) preview_->hide();
	if (!(f & KPrinter::Properties)) propbutton_->hide();

	// update pages with current options
	options_ = printer_->options();
	QListIterator<KPrintDialogPage>	pit(pages_);
	for (;pit.current();++pit)
		pit.current()->setOptions(options_);

	// update global options with KPrinter object
	if (printer_->outputToFile())
	{
		outputtofile_->setChecked(true);
		outputfilename_->setText(printer_->outputFileName());
	}
	int	p = printerslist_->findRef(printer_->implementation()->findPrinter(printer_->searchName()));
	if (p != -1)
		dest_->setCurrentItem(p);

	if (options_["qt-preview"] == "1") preview_->setChecked(true);

	printerSelected(dest_->currentItem());
}

bool KPrintDialog::printerSetup(KPrinter *printer, QWidget *parent)
{
	KPrintDialog	dlg(printer,parent);
	dlg.resize(10,10);
	if (dlg.exec())
	{
		// do something
		return true;
	}
	return false;
}

void KPrintDialog::setValidPrinter(bool v)
{
	setPrinterType("");
	setPrinterLocation("");
	setPrinterState("");
	setPrinterComment("");
	propbutton_->setEnabled(v);
	okbutton_->setEnabled(v);
	defaultbutton_->setEnabled(v);
}

void KPrintDialog::printerSelected(int index)
{
	printer_->implementation()->printerSelected(this,index);
}

void KPrintDialog::propertyClicked()
{
	if (dest_->count() == 0 || dest_->currentItem() < 0)
	{
		QMessageBox::critical(this,i18n("Error"),i18n("No printer selected !"),QMessageBox::Ok|QMessageBox::Default,0);
		return;
	}

	int	index = (outputtofile_->isChecked() ? -1 : dest_->currentItem());
	KPrinterPropertyDialog	*dlg = printer_->implementation()->buildPropertyDialog(this, index);
	if (dlg)
	{
		dlg->exec();
		delete dlg;
	}
}

void KPrintDialog::pageSelected(QWidget *w)
{
	if (currentpage_) currentpage_->getOptions(options_,true);
	currentpage_ = (KPrintDialogPage*)w;
	if (currentpage_) currentpage_->setOptions(options_);
}

void KPrintDialog::done(int result)
{
	if (result == Accepted)
	{
		// if no printer, just do nothing
		if (dest_->count() == 0 || dest_->currentItem() < 0)
		{
			QMessageBox::critical(this,i18n("Error"),i18n("No printer selected !"),QMessageBox::Ok|QMessageBox::Default,0);
			return;
		}

		// start with KPrinter object's options
		//OptionSet	opts = printer_->options();

		// Try without default KPrinter options as normally
		// everything should be set up by the dialog
		OptionSet	opts;

		// retrieve options from KPrintDialog pages
		QListIterator<KPrintDialogPage>	it(pages_);
		for (;it.current();++it)
			it.current()->getOptions(opts,true);
		// add other options
		KPrinterItem	*item(0);
		if (outputtofile_->isChecked())
		{
			if (!checkOutputFile()) return;
			printer_->setOutputToFile(true);
			printer_->setOutputFileName(outputfilename_->text());
			item = printer_->implementation()->filePrinter();
		}
		else
		{
			QString	msg;
			if (!printer_->implementation()->checkPrinter(dest_->currentItem(),msg))
			{
				QMessageBox::critical(this,i18n("Error"),msg,QMessageBox::Ok|QMessageBox::Default,0);
				return;
			}
			item = printerslist_->at(dest_->currentItem());
			printer_->setPrinterName(item->printerName());
			printer_->setSearchName(item->name());
			printer_->setOutputToFile(false);
			opts["qt-preview"] = QString::fromLatin1(preview_->isChecked() ? "1" : "0");
		}
		opts.merge((item->isEdited() ? item->editedOptions() : item->defaultOptions()));

		// update KPrinter object
		printer_->setOptions(opts);

		QDialog::done(result);
	}
	else
		QDialog::done(result);
}

bool KPrintDialog::checkOutputFile()
{
	bool	value(false);
	if (outputfilename_->text().isEmpty())
		QMessageBox::critical(this,i18n("Error"),i18n("The output filename is empty !"),QMessageBox::Ok|QMessageBox::Default,0);
	else
	{
		QFileInfo	f(outputfilename_->text());
		if (f.exists())
		{
			if (f.isWritable())
				value = (QMessageBox::warning(this,i18n("Warning"),i18n("File \"%1\" already exists. Overwrite ?").arg(f.absFilePath()),QMessageBox::Yes|QMessageBox::Default,QMessageBox::No) == QMessageBox::Yes);
			else
				QMessageBox::critical(this,i18n("Error"),i18n("You don't have write permissions to this file !"),QMessageBox::Ok|QMessageBox::Default,0);
		}
		else
		{
			if (QFileInfo(f.dirPath(true)).isWritable())
				value = true;
			else
				QMessageBox::critical(this,i18n("Error"),i18n("You don't have write permissions in that directory !"),QMessageBox::Ok|QMessageBox::Default,0);
		}
	}
	return value;
}

void KPrintDialog::outputFileBrowseClicked()
{
	QString	filename = KFileDialog::getOpenFileName(outputfilename_->text(),i18n("*.ps|PostScript document (*.ps)\n*|All files"),this,i18n("Print to file"));
	if (!filename.isEmpty())
		outputfilename_->setText(filename);
}

void KPrintDialog::setDefaultClicked()
{
	if (dest_->count() == 0 || dest_->currentItem() < 0)
	{
		QMessageBox::critical(this,i18n("Error"),i18n("No printer selected !"),QMessageBox::Ok|QMessageBox::Default,0);
		return;
	}

	printer_->implementation()->setDefaultPrinter(this,(outputtofile_->isChecked() ? -1 : dest_->currentItem()));
}

void KPrintDialog::optionsClicked()
{
	if (printer_->implementation()->configure(this))
	{
		reloadPrinters();
		printerSelected(dest_->currentItem());
	}
}
#include "kprintdialog.moc"

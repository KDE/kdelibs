#include "kcupsdialogimpl.h"
#include "ipprequest.h"
#include "cupsinfos.h"
#include "kprintdialog.h"
#include "kprinterpropertydialog.h"
#include "kprinter.h"
#include "kcupsoptiondlg.h"

#include "kcupsgeneralpage.h"
#include "kcupsppdpage.h"
#include "kprintsizecolorpage.h"
#include "kcupsimagepage.h"
#include "kcupstextpage.h"
#include "kcupshpgl2page.h"

#ifdef HAVE_CONFIG
#include <config.h>
#endif

#ifndef CUPSCONFIGDIR
#define CUPSCONFIGDIR "/etc/cups"
#endif

#include <qfile.h>
#include <qtextstream.h>
#include <qfileinfo.h>
#include <qmessagebox.h>

#include <kapp.h>
#include <klocale.h>
#include <kiconloader.h>

#include <stdlib.h>
#include <cups/cups.h>

void ppdMarkOptions(global_ppd_file_t *ppd, const OptionSet& opts);
QPixmap iconForType(int type, int state = KPrinterItem::Idle);

//***************************************************************************************************

KCupsDialogImpl::KCupsDialogImpl(KPrinter *pr, QObject *parent, const char *name)
	: KDialogImpl(pr,parent,name)
{
	request_ = 0;
	ppd_ = 0;
}

KCupsDialogImpl::~KCupsDialogImpl()
{
	closePPD();
	closeRequest();
}

KPrinterItemList* KCupsDialogImpl::printersList()
{
	KPrinterItemListIterator	it(printers_);
	for (;it.current();++it)
		it.current()->setDiscarded(true);

	loadServerPrinters();
	loadOptionsFiles();

	for (uint i=0;i<printers_.count();i++)
	{
		if (printers_.at(i)->isDiscarded())
		{
			printers_.remove(i);
			i--;
		}
	}

	return &printers_;
}

void KCupsDialogImpl::printerSelected(KPrintDialog *dlg, int index)
{
	if (index < 0 || index >= (int)(printers_.count()))
	{
		dlg->setValidPrinter(false);
		return;
	}

	IppRequest	req;
	KPrinterItem	*printer = printers_.at(index);
	QStringList	keys;
	QString		uri;

	req.setOperation(IPP_GET_PRINTER_ATTRIBUTES);
	uri = QString("ipp://%1:%2/printers/%3").arg(CupsInfos::self()->host()).arg(CupsInfos::self()->port()).arg(printer->printerName());
	req.addURI(IPP_TAG_OPERATION,"printer-uri",uri);
	keys.append("printer-location");
	keys.append("printer-state");
	keys.append("printer-info");
	keys.append("printer-make-and-model");
	req.addKeyword(IPP_TAG_OPERATION,"requested-attributes",keys);

	if (req.doRequest("/printers/"))
	{
		dlg->setValidPrinter(true);
		QString	value;
		if (req.text("printer-info",value)) dlg->setPrinterType(value);
		if (req.text("printer-location",value)) dlg->setPrinterLocation(value);
		if (req.text("printer-make-and-model",value)) dlg->setPrinterComment(value);
		int	n;
		if (req.enumvalue("printer-state",n))
			switch(n)
			{
				case IPP_PRINTER_IDLE: dlg->setPrinterState(i18n("Printer idle")); break;
				case IPP_PRINTER_PROCESSING: dlg->setPrinterState(i18n("Processing...")); break;
				case IPP_PRINTER_STOPPED: dlg->setPrinterState(i18n("Stopped")); break;
				default: dlg->setPrinterState(i18n("Unknown")); break;
			}
	}
	else
	{
		dlg->setValidPrinter(false);
	}
}

void KCupsDialogImpl::processRequest(IppRequest& req)
{
	ipp_attribute_t	*attr = req.first();
	KPrinterItem	*printer = new KPrinterItem();
	while (attr)
	{
		QString	attrname(attr->name);
		if (attrname == "printer-name")
		{
			QString	value = QString::fromLocal8Bit(attr->values[0].string.text);
			printer->setName(value);
			printer->setDisplayName(value);
			printer->setPrinterName(value);
		}
		else if (attrname == "printer-type")
		{
			int	value = attr->values[0].integer;
			printer->addType(KPrinterItem::Valid);
			printer->addType((value & CUPS_PRINTER_CLASS ? KPrinterItem::Class : KPrinterItem::Printer));
			printer->addType((value & CUPS_PRINTER_REMOTE ? KPrinterItem::Remote : KPrinterItem::Local));
			if (value & CUPS_PRINTER_IMPLICIT) printer->addType(KPrinterItem::Implicit);
		}
		else if (attrname == "printer-state")
		{
			switch (attr->values[0].integer)
			{
				case IPP_PRINTER_IDLE: printer->setState(KPrinterItem::Idle); break;
				case IPP_PRINTER_PROCESSING: printer->setState(KPrinterItem::Processing); break;
				case IPP_PRINTER_STOPPED: printer->setState(KPrinterItem::Stopped); break;
			}
		}
		if (attrname.isEmpty() || attr == req.last())
		{
			printer->setPixmap(iconForType(printer->type(),printer->state()));
			addPrinter(printer);
			printer = new KPrinterItem();
		}
		attr = attr->next;
	}
}

void KCupsDialogImpl::loadServerPrinters()
{
	IppRequest	req;
	QStringList	keys;

	// get printers
	req.setOperation(CUPS_GET_PRINTERS);
	keys.append("printer-name");
	keys.append("printer-type");
	keys.append("printer-state");
	req.addKeyword(IPP_TAG_OPERATION,"requested-attributes",keys);

	if (req.doRequest("/printers/"))
		processRequest(req);

	// get classes
	req.init();
	req.setOperation(CUPS_GET_CLASSES);
	req.addKeyword(IPP_TAG_OPERATION,"requested-attributes",keys);

	if (req.doRequest("/classes/"))
		processRequest(req);
}

void KCupsDialogImpl::loadOptionsFile(const QString& filename)
{
	QFile	f(filename);
	if (f.exists() && f.open(IO_ReadOnly))
	{
		QTextStream	t(&f);
		QString	line;
		int	p(-1);
		KPrinterItem	*printer, *other;
		while (!t.eof())
		{
			line = t.readLine().stripWhiteSpace();
			QStringList	words = QStringList::split(' ', line, false);
			if (words.count() < 2) continue;
			printer = new KPrinterItem();
			printer->setName(words[1]);
			if ((p=words[1].find('/')) != -1)
			{
				printer->setPrinterName(words[1].left(p));
				printer->setDisplayName(i18n("%1 on %2").arg(words[1].right(words[1].length()-p-1)).arg(printer->printerName()));
			}
			else
			{
				printer->setPrinterName(words[1]);
				printer->setDisplayName(words[1]);
			}
			if ((other=findPrinter(printer->printerName())) != NULL && !other->isDiscarded())
			{
				printer->setType(other->type());
				printer->setPixmap(other->pixmap());
			}
			if (! (printer->type() & KPrinterItem::Valid))
				printer->setPixmap(UserIcon("kdeprint_printer_defect"));
			for (uint i=2;i<words.count();i++)
			{
				if ((p=words[i].find('=')) != -1)
					printer->addDefaultOption(words[i].left(p),words[i].right(words[i].length()-p-1));
				else
					printer->addDefaultOption(words[i],QString::fromLatin1(""));
			}
			addPrinter(printer);

			if (words[0] == "Default")
			{
				printer = findPrinter(words[1]);
				setDefault(printer);
			}
		}
	}
}

void KCupsDialogImpl::loadOptionsFiles()
{
	QFileInfo	global(QString::fromLatin1(CUPSCONFIGDIR)+"/lpoptions"), local(getenv("HOME")+QString::fromLatin1("/.lpoptions"));
	if (!checktime_.isValid() || global.lastModified() > checktime_ || local.lastModified() > checktime_)
	{
		checktime_ = QMAX(global.lastModified(),local.lastModified());
		KPrinterItemListIterator	it(printers_);
		for (;it.current();++it)
			it.current()->clear();
		loadOptionsFile(global.absFilePath());
		loadOptionsFile(local.absFilePath());
	}
	else
		for (printers_.first();printers_.current();printers_.next())
			if (printers_.current()->name().find('/') != -1)
				printers_.current()->setDiscarded(false);
}

bool KCupsDialogImpl::saveOptionsFile(const QString& filename)
{ // save "default" options of all printers contained in "printers_"
  // list. If saving a printer configuration, the default options should
  // be updated before (and edited options removed).
	QFile	f(filename);
	if (f.open(IO_WriteOnly))
	{
		QTextStream	t(&f);
		KPrinterItemListIterator	it(printers_);
		for (;it.current();++it)
		{
			if (it.current()->defaultOptions().count() == 0 && !it.current()->isDefault())
				continue;	// do not save this entry as it doesn't have any specific
						// options, and is not the default printer.
			t << (it.current()->isDefault() ? "Default" : "Dest") << ' ';
			t << it.current()->name() << ' ';
			for (QMap<QString,QString>::ConstIterator oit = it.current()->defaultOptions().begin(); oit != it.current()->defaultOptions().end(); ++oit)
			{
				if (oit.data().isEmpty())
					t << oit.key() << ' ';
				else
					t << oit.key() << '=' << oit.data() << ' ';
			}
			t << endl;
		}
		return true;
	}
	QMessageBox::critical(kapp->mainWidget(),i18n("Save error"),i18n("Unable to save printers options to \"%1\" !").arg(filename),QMessageBox::Ok|QMessageBox::Default,0);
	return false;
}

void KCupsDialogImpl::slotSaveRequested(KPrinterItem *pr, const OptionSet& opts)
{
	// update printer item structure
	pr->setDefaultOptions(opts);
	pr->setEditedOptions(OptionSet());
	pr->setEdited(false);

	// file to save
	QString	fname = getenv("HOME") + QString::fromLatin1("/.lpoptions");

	// save the printer list
qDebug("saving options to %s",fname.latin1());
	saveOptionsFile(fname);
}

void KCupsDialogImpl::setDefaultPrinter(KPrintDialog *dlg, int index)
{
	KDialogImpl::setDefaultPrinter(dlg,index);
	QString	fname = getenv("HOME") + QString::fromLatin1("/.lpoptions");
	saveOptionsFile(fname);
}

void KCupsDialogImpl::closePPD()
{
	if (ppd_)
	{
		globalPpdClose(ppd_);
		ppd_ = 0;
	}
}

void KCupsDialogImpl::closeRequest()
{
	if (request_)
	{
		delete request_;
		request_ = 0;
	}
}

KPrinterPropertyDialog* KCupsDialogImpl::buildPropertyDialog(KPrintDialog *dlg, int index)
{
	closePPD();
	closeRequest();

	// create dialog
	KPrinterPropertyDialog	*pdlg = new KPrinterPropertyDialog(true,dlg);
	connect(pdlg,SIGNAL(saveRequested(KPrinterItem*,const OptionSet&)),SLOT(slotSaveRequested(KPrinterItem*,const OptionSet&)));
	KPrinterItem	*printer(0);

	if (index >= 0)
	{
		printer = printers_.at(index);
		request_ = new IppRequest();
		QString	str;
		bool hasPPD((printer->type() & KPrinterItem::Printer) && (printer->type() & KPrinterItem::Local) && !(printer->type() & KPrinterItem::Implicit));

		str = (hasPPD ? QString::fromLatin1(cupsGetPPD(printer->printerName().latin1())) : QString::null);
		if (!str.isEmpty())
		{
			ppd_ = globalPpdOpenFile(str.latin1());
			::unlink(str.latin1());
			if (ppd_)
			{
				// update PPD from printer options
				globalPpdMarkDefaults(ppd_);
				ppdMarkOptions(ppd_,(printer->isEdited() ? printer->editedOptions() : printer->defaultOptions()));
			}
		}
		request_->setOperation(IPP_GET_PRINTER_ATTRIBUTES);
		str = QString("ipp://%1:%2/printers/%3").arg(CupsInfos::self()->host()).arg(CupsInfos::self()->port()).arg(printer->printerName());
		request_->addURI(IPP_TAG_OPERATION,QString::fromLatin1("printer-uri"),str);
		if (!request_->doRequest("/printers/"))
			closeRequest();

		// General page
		pdlg->addPage(new KCupsGeneralPage(ppd_,request_,pdlg));

		// additional pages if necessary
		if (printer_->applicationType() == KPrinter::StandAlone)
		{
			pdlg->addPage(new KCupsImagePage(ppd_,request_,pdlg));
			pdlg->addPage(new KCupsTextPage(ppd_,request_,pdlg));
			pdlg->addPage(new KCupsHPGL2Page(ppd_,request_,pdlg));
		}

		if (ppd_) pdlg->addPage(new KCupsPPDPage(ppd_,request_,pdlg));

	}
	else
	{
		printer = fileprinter_;

		// add the only relevant page for file printer
		pdlg->addPage(new KPrintSizeColorPage(pdlg));
	}

	// set printer
	pdlg->setPrinter(printer);

	return pdlg;
}

bool KCupsDialogImpl::configure(KPrintDialog *dlg)
{
	bool	flag = KCupsOptionDlg::configure(dlg);
	if (flag)
		checktime_ = QDateTime();
	return flag;
}

//***************************************************************************************************

void ppdMarkOptions(global_ppd_file_t *ppd, const OptionSet& opts)
{
	if (!ppd || !ppd->ppd) return;
	for (QMap<QString,QString>::ConstIterator it = opts.begin(); it != opts.end(); ++it)
		globalPpdMarkOption(ppd,it.key().latin1(),it.data().latin1());
}

QPixmap iconForType(int type, int state)
{
	QString	name("kdeprint_printer");
	bool	isclass((type&KPrinterItem::Class) || (type&KPrinterItem::Implicit));
	bool	isremote((type&KPrinterItem::Remote));
	if (isclass) name.append("_class");
	if (!isclass && isremote) name.append("_remote");
	if (state == KPrinterItem::Stopped) name.append("_stopped");
	return UserIcon(name);
}

#include "kpdqdialogimpl.h"
#include "pdqclass.h"
#include "pdqinfos.h"
#include "kprintdialog.h"
#include "kprinter.h"
#include "kprinterpropertydialog.h"
#include "kpdqoptiondlg.h"

#include "kprintsizecolorpage.h"
#include "kpdqpage.h"

#include <qfileinfo.h>
#include <klocale.h>
#include <stdlib.h>

#include "kpdqview.h"
#include <qtabdialog.h>

KPdqDialogImpl::KPdqDialogImpl(KPrinter *pr, QObject *parent, const char *name)
: KDialogImpl(pr,parent,name)
{
	pdq_ = new PDQMain();

	standardpages_ = KPrinter::CopiesPage|KPrinter::SizeColorPage;
}

KPdqDialogImpl::~KPdqDialogImpl()
{
	if (pdq_) delete pdq_;
}

KPrinterItemList* KPdqDialogImpl::printersList()
{
	reload();
	return &printers_;
}

void KPdqDialogImpl::printerSelected(KPrintDialog *dlg, int index)
{
	if (index < 0 || index >= (int)(printers_.count()))
	{
		dlg->setValidPrinter(false);
		return;
	}

	KPrinterItem	*pr = printers_.at(index);
	PDQPrinter	*ppr = pdq_->printer(pr->name());
	PDQInterface	*in = (ppr ? pdq_->interface(ppr->interface_) : 0);
	if (!ppr) return;

	dlg->setValidPrinter(true);
	dlg->setPrinterState(i18n("Printer idle"));
	dlg->setPrinterLocation(ppr->location_);
	dlg->setPrinterComment(ppr->model_);
	dlg->setPrinterType((in ? in->name() : QString::fromLatin1("")));
}

void KPdqDialogImpl::slotSaveRequested(KPrinterItem *item, const OptionSet& opts)
{
	if (!item) return;

	PDQPrinter	*npr = printerWithOptions(item->name(),opts);
	PDQMain		*tmp_ = new PDQMain();
	QString		filename = getenv("HOME") + QString::fromLatin1("/.printrc");

	tmp_->parseFile(filename,false);
	tmp_->setPrinter(npr);
	tmp_->saveFile(filename);

	item->setDefaultOptions(opts);
	item->setEditedOptions(OptionSet());
	item->setEdited(false);

	delete tmp_;
}

PDQPrinter* KPdqDialogImpl::printerWithOptions(const QString& prname, const OptionSet& opts)
{
	PDQPrinter	*pr_ = pdq_->printer(prname);
	PDQDriver	*dr = pdq_->driver(pr_->driver_);
	PDQInterface	*in = pdq_->interface(pr_->interface_);
	PDQPrinter	*pr = pr_->duplicate();

	if (dr)
	{
		QListIterator<PDQOption>	oit(dr->options_);
		QString	value("");
		pr->driver_options_.clear();
		for (;oit.current();++oit)
		{
			if (opts.contains(oit.current()->name())) value = opts[oit.current()->name()];
			else if (!oit.current()->default_choice_.isEmpty()) value = oit.current()->default_choice_;
			else if (oit.current()->choices_.count() > 0) value = oit.current()->choices_.first()->name();
			pr->driver_options_.append(value);
		}

		QListIterator<PDQArgument>	ait(dr->arguments_);
		value = "";
		pr->driver_args_.clear();
		for (;ait.current();++ait)
		{
			if (opts.contains(ait.current()->name())) value = opts[ait.current()->name()];
			else value = ait.current()->defvalue_;
			if (!value.isEmpty()) pr->driver_args_[ait.current()->name()] = value;
		}
	}
	if (in)
	{
		QListIterator<PDQOption>	oit(in->options_);
		QString	value("");
		pr->interface_options_.clear();
		for (;oit.current();++oit)
		{
			if (opts.contains(oit.current()->name())) value = opts[oit.current()->name()];
			else if (!oit.current()->default_choice_.isEmpty()) value = oit.current()->default_choice_;
			else if (oit.current()->choices_.count() > 0) value = oit.current()->choices_.first()->name();
			pr->interface_options_.append(value);
		}

		QListIterator<PDQArgument>	ait(in->arguments_);
		value = "";
		pr->interface_args_.clear();
		for (;ait.current();++ait)
		{
			if (opts.contains(ait.current()->name())) value = opts[ait.current()->name()];
			else value = ait.current()->defvalue_;
			if (!value.isEmpty()) pr->interface_args_[ait.current()->name()] = value;
		}
	}

	return pr;
}

KPrinterPropertyDialog* KPdqDialogImpl::buildPropertyDialog(KPrintDialog *dlg, int index)
{
	KPrinterPropertyDialog	*pdlg = new KPrinterPropertyDialog(true,dlg);
	connect(pdlg,SIGNAL(saveRequested(KPrinterItem*,const OptionSet&)),SLOT(slotSaveRequested(KPrinterItem*,const OptionSet&)));
	KPrinterItem	*printer(0);

	if (index >= 0)
	{
		printer = printers_.at(index);

		PDQPrinter	*pr = pdq_->printer(printer->name());
		PDQDriver	*dr = pdq_->driver(pr->driver_);
		PDQInterface	*in = pdq_->interface(pr->interface_);

		pdlg->addPage(new KPdqPage(pr,dr,in,pdlg));
	}
	else
	{
		printer = fileprinter_;

		// add the only relevant page for file printer
		pdlg->addPage(new KPrintSizeColorPage(pdlg));
	}

	// set printer
	pdlg->setPrinter(printer);
	pdlg->resize(450,500);

	return pdlg;
}

void KPdqDialogImpl::setDefaultPrinter(KPrintDialog *dlg, int index)
{
	KDialogImpl::setDefaultPrinter(dlg,index);
	if (index < 0 || index >= (int)(printers_.count())) return;

	KPrinterItem	*item = printers_.at(index);
	if (item && !item->name().isEmpty())
	{
		PDQMain	*tmp_ = new PDQMain();
		QString	filename = getenv("HOME") + QString::fromLatin1("/.printrc");

		tmp_->parseFile(filename,false);
		tmp_->setDefaultPrinter(item->name());
		tmp_->saveFile(filename);

		delete tmp_;
	}
}

void KPdqDialogImpl::reload()
{
	QFileInfo	gl(PdqInfos::self()->pdqGlobalRcFile()), loc(getenv("HOME")+QString::fromLatin1("/.printrc"));

	if (checktime_.isValid() && checktime_ >= QMAX(gl.lastModified(),loc.lastModified()))
		return;
	checktime_ = QMAX(gl.lastModified(),loc.lastModified());

	pdq_->init();
	pdq_->parseFile(gl.absFilePath(),true);
	pdq_->parseFile(loc.absFilePath(),true);

	// discard all printers
	KPrinterItemListIterator	it(printers_);
	for (;it.current();++it)
		it.current()->setDiscarded(true);

	QDictIterator<PDQPrinter>	pit(pdq_->printers_);
	for (;pit.current();++pit)
	{
		PDQPrinter	*pr = pdq_->printer(pit.currentKey());
		PDQDriver	*dr = pdq_->driver(pr->driver_);
		PDQInterface	*in = pdq_->interface(pr->interface_);

		KPrinterItem	*item = new KPrinterItem();
		item->setName(pr->name());
		item->setDisplayName(pr->name());
		item->setPrinterName(pr->name());
		item->addType(KPrinterItem::Printer);

		if (dr)
		{
			// driver arguments
			for (QMap<QString,QString>::ConstIterator it = pr->driver_args_.begin(); it != pr->driver_args_.end(); ++it)
				item->addDefaultOption(it.key(), it.data());
			// driver option
			for (uint i=0;i<pr->driver_options_.count();i++)
				item->addDefaultOption(dr->options_.at(i)->name(),pr->driver_options_[i]);
		}
		if (in)
		{
			// interface arguments
			for (QMap<QString,QString>::ConstIterator it = pr->interface_args_.begin(); it != pr->interface_args_.end(); ++it)
				item->addDefaultOption(it.key(), it.data());
			// interface option
			for (uint i=0;i<pr->interface_options_.count();i++)
				item->addDefaultOption(in->options_.at(i)->name(),pr->interface_options_[i]);
		}

		addPrinter(item);
	}
	if (!pdq_->defprinter_.isEmpty())
	{
		KPrinterItem	*pr = findPrinter(pdq_->defprinter_);
		setDefault(pr);
	}

	// remove discarded
	for (uint i=0;i<printers_.count();i++)
	{
		if (printers_.at(i)->isDiscarded())
		{
			printers_.remove(i);
			i--;
		}
	}
}

bool KPdqDialogImpl::configure(KPrintDialog *dlg)
{
	bool	flag = KPdqOptionDlg::configure(dlg);
	if (flag)
	{
		checktime_ = QDateTime();	// set check time to invalid date to force reload
		reload();
	}
	return flag;
}
#include "kpdqdialogimpl.moc"

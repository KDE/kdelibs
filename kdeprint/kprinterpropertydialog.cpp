#include "kprinterpropertydialog.h"
#include "kprintdialogpage.h"
#include "kprinteritem.h"

#include <klocale.h>

KPrinterPropertyDialog::KPrinterPropertyDialog(bool hasSaveBtn, QWidget *parent, const char *name)
: QTabDialog(parent,name,true)
{
	pages_.setAutoDelete(false);
	printer_ = 0;
	currentpage_ = 0;

	setOkButton(i18n("OK"));
	setCancelButton(i18n("Cancel"));
	if (hasSaveBtn)
	{
		setDefaultButton(i18n("Save"));
		connect(this,SIGNAL(defaultButtonPressed()),SLOT(saveClicked()));
	}
	connect(this,SIGNAL(currentChanged(QWidget*)),SLOT(pageSelected(QWidget*)));

	resize(10,10);
}

KPrinterPropertyDialog::~KPrinterPropertyDialog()
{
}

void KPrinterPropertyDialog::pageSelected(QWidget *w)
{
	if (currentpage_) currentpage_->getOptions(options_,true);
	currentpage_ = (KPrintDialogPage*)w;
	if (currentpage_) currentpage_->setOptions(options_);
}

void KPrinterPropertyDialog::setPrinter(KPrinterItem *printer)
{
	printer_ = printer;
	options_ = (printer_->isEdited() ? printer_->editedOptions() : printer_->defaultOptions());
debug("**********************");
debug("starting options...");
options_.dump();
	QListIterator<KPrintDialogPage>	it(pages_);
	for (;it.current();++it)
		it.current()->setOptions(options_);
	setCaption(i18n("Configuration of %1").arg(printer_->displayName()));
}

void KPrinterPropertyDialog::done(int result)
{
	if (result == Accepted)
	{
		OptionSet	opts;
		if (retrieveOptions(opts,false))
		{
			printer_->setEditedOptions(opts);
			printer_->setEdited(true);
debug("**********************");
debug("ending options...");
opts.dump();
			QTabDialog::done(result);
		}
	}
	else
		QTabDialog::done(result);
}

bool KPrinterPropertyDialog::retrieveOptions(OptionSet& opts, bool incldef)
{
	// first synchronize all pages (this also shows necessary warning boxes).
	if (!syncOptions()) return false;

	// retrieve options from all pages. Stop if an invalid page is found.
	QListIterator<KPrintDialogPage>	it(pages_);
	for (;it.current();++it)
		it.current()->getOptions(opts,incldef);
	return true;
}

bool KPrinterPropertyDialog::syncOptions()
{
	if (currentpage_) currentpage_->getOptions(options_,true);
	QListIterator<KPrintDialogPage>	it(pages_);
	bool	valid(true);
	for (;it.current();++it)
	{
		it.current()->setOptions(options_);	// this command should popup all
							// necessary warning dialogs !!!
		valid = valid && it.current()->isValid();
	}
	return valid;
}

void KPrinterPropertyDialog::saveClicked()
{
	OptionSet	opts;
	if (retrieveOptions(opts,false))	// emit "save" only if all pages are valid.
		emit saveRequested(printer_,opts);
}

void KPrinterPropertyDialog::addPage(KPrintDialogPage *p)
{
	pages_.append(p);
	addTab(p,p->pageTitle());
}

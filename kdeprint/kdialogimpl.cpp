#include "kdialogimpl.h"

#include "kprintdialogpage.h"
#include "kprintcopiespage.h"
#include "kprintsizecolorpage.h"
#include "kprinter.h"

#include <klocale.h>
#include <kiconloader.h>

KDialogImpl::KDialogImpl(KPrinter *printer, QObject *parent, const char *name)
: QObject(parent, name), printer_(printer)
{
	pages_.setAutoDelete(false);
	printers_.setAutoDelete(true);

	dialogflags_ = KPrinter::All;
	standardpages_ = KPrinter::CopiesPage;

	fileprinter_ = new KPrinterItem();
	fileprinter_->setPrinterName(QString::fromLatin1("file"));
	fileprinter_->setName(QString::fromLatin1("file"));
	fileprinter_->setDisplayName(i18n("Print to file"));
}

KDialogImpl::~KDialogImpl()
{
	delete fileprinter_;
}

KPrinterItem* KDialogImpl::findPrinter(const QString& name)
{
	KPrinterItemListIterator	it(printers_);
	for (;it.current();++it)
		if (it.current()->name() == name) return it.current();
	return NULL;
}

void KDialogImpl::addPrinter(KPrinterItem *printer)
{
	if (printer->pixmap().isNull())
		printer->setPixmap(UserIcon("kdeprint_printer"));
	KPrinterItem	*item = findPrinter(printer->name());
	if (item)
	{
		bool	isDef(item->isDefault());
		item->copy(*printer);
		if (isDef) setDefault(item);
		delete printer;
	}
	else
		printers_.inSort(printer);
}

void KDialogImpl::setDefault(KPrinterItem *printer)
{
	KPrinterItemListIterator	it(printers_);
	for (;it.current();++it)
		it.current()->setDefault(false);
	if (printer) printer->setDefault(true);
}

void KDialogImpl::addDialogPage(KPrintDialogPage *page)
{
	pages_.append(page);
}

QList<KPrintDialogPage>* KDialogImpl::dialogPages()
{
	// add default pages
	if (standardpages_ & KPrinter::CopiesPage)
		pages_.insert(0,new KPrintCopiesPage(0));
	if (standardpages_ & KPrinter::SizeColorPage)
		pages_.insert(1,new KPrintSizeColorPage(0));

	return &pages_;
}

bool KDialogImpl::checkPrinter(int, QString&)
{
	return true;
}

void KDialogImpl::setDefaultPrinter(KPrintDialog*, int index)
{
	if (index >= 0 && index < (int)(printers_.count()))
		setDefault(printers_.at(index));
}

void KDialogImpl::setGlobalOption(const QString& key, const QString& value)
{
	// do not propagate options starting with "qt-..." as they are not
	// printer-specific (global options).
	if (key.left(3) == "qt-") return;

	// force printer listing if not done yet (or reload needed)
	if (printers_.count() == 0) printersList();
	KPrinterItemListIterator	it(printers_);
	for (;it.current();++it)
	{
		// add the option to the edited set, otherwise you'll save
		// this option for all printers when trying to save !!
		it.current()->addEditedOption(key,value);
		it.current()->setEdited(true);
	}

	// update also "file" printer
	fileprinter_->addEditedOption(key,value);
	fileprinter_->setEdited(true);
}

bool KDialogImpl::configure(KPrintDialog*)
{
	return false;
}

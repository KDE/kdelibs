#include "kcupsprinterimpl.h"
#include "kprinter.h"
#include "driver.h"
#include "kmfactory.h"
#include "kmmanager.h"

#include <qfile.h>
#include <cups/cups.h>
#include <stdlib.h>

static int mapToCupsOptions(const QMap<QString,QString>& opts, cups_option_t **options);

//******************************************************************************************************

KCupsPrinterImpl::KCupsPrinterImpl(QObject *parent, const char *name)
: KPrinterImpl(parent,name)
{
}

KCupsPrinterImpl::~KCupsPrinterImpl()
{
}

bool KCupsPrinterImpl::printFiles(KPrinter *printer, const QStringList& files)
{
	// check printer object
	if (!printer) return false;

	// convert Map to CUPS options
	cups_option_t	*options(0);
	int		num_options(0);
	num_options = mapToCupsOptions(printer->options(), &options);

	// convert QStringList to char array
	char 	**cfiles = (char**)malloc(sizeof(char*)*files.count());
	int 		index(0);
	for (QStringList::ConstIterator it=files.begin(); it!=files.end(); ++it, index++)
		cfiles[index] = strdup(QFile::encodeName(*it).data());

	// print files
	QString	jobTitle = printer->docName();
	if (jobTitle.isEmpty()) jobTitle = "KDE Print System";
	int 	jobID = cupsPrintFiles(printer->printerName().local8Bit().data(),files.count(),(const char**)cfiles,jobTitle.local8Bit().data(),num_options,options);

	// clear allocated memory
	for (uint i=0;i<files.count();i++)
		free(cfiles[i]);
	free(cfiles);

	// return status
	return (jobID > 0);
}

void KCupsPrinterImpl::preparePrinting(KPrinter *printer)
{
	// process orientation
	QString	o = printer->option("orientation-requested");
	printer->setOption("kde-orientation",(o == "4" || o == "5" ? "Landscape" : "Portrait"));
	// if it's a Qt application, then convert orientation as it will be handled by Qt directly
	if (printer->applicationType() == KPrinter::Dialog)
		printer->setOption("orientation-requested",(o == "5" || o == "6" ? "6" : "3"));

	// translate copies number
	if (!printer->option("kde-copies").isEmpty()) printer->setOption("copies",printer->option("kde-copies"));

	// page ranges are handled by CUPS, so application should print all pages
	if (printer->pageSelection() == KPrinter::SystemSide)
	{ // Qt => CUPS
		// translations
		if (!printer->option("kde-range").isEmpty())
			printer->setOption("page-ranges",printer->option("kde-range"));
		if (!printer->option("kde-pageorder").isEmpty())
			printer->setOption("OutputOrder",printer->option("kde-pageorder"));
		o = printer->option("kde-pageset");
		if (!o.isEmpty() && o != "0")
			printer->setOption("page-set",(o == "1" ? "odd" : "even"));
		printer->setOption("multiple-document-handling",(printer->option("kde-collate") == "Collate" ? "separate-documents-collated-copies" : "separate-documents-uncollated-copies"));
	}
	else
	{ // No translation needed (but range => (from,to))
		QString range = printer->option("kde-range");
		if (!range.isEmpty())
		{
			QSize	s = rangeToSize(range);
			printer->setOption("kde-from",QString::number(s.width()));
			printer->setOption("kde-to",QString::number(s.height()));
		}
	}

	// page size -> try to find page size from driver file
	KMManager	*mgr = KMFactory::self()->manager();
	DrMain	*driver = mgr->loadPrinterDriver(mgr->findPrinter(printer->printerName()), false);
	if (driver)
	{
		QString	psname = printer->option("PageSize");
		if (psname.isEmpty())
		{
			DrListOption	*opt = (DrListOption*)driver->findOption("PageSize");
			if (opt) psname = opt->get("default");
		}
		if (!psname.isEmpty())
		{
			printer->setOption("kde-pagesize",QString::number((int)pageNameToPageSize(psname)));
			DrPageSize	*ps = driver->findPageSize(psname);
			if (ps)
			{
				printer->setRealPageSize(ps->pageSize());
				printer->setMargins(ps->margins());
			}
		}
		delete driver;
	}

	KPrinterImpl::preparePrinting(printer);
}

void KCupsPrinterImpl::broadcastOption(const QString& key, const QString& value)
{
	KPrinterImpl::broadcastOption(key,value);
	if (key == "kde-orientation")
		KPrinterImpl::broadcastOption("orientation-requested",(value == "Landscape" ? "4" : "3"));
	else if (key == "kde-pagesize")
	{
		QString	pagename = QString::fromLatin1(pageSizeToPageName((KPrinter::PageSize)value.toInt()));
		KPrinterImpl::broadcastOption("PageSize",pagename);
		// simple hack for classes
		KPrinterImpl::broadcastOption("media",pagename);
	}
}

//******************************************************************************************************

static int mapToCupsOptions(const QMap<QString,QString>& opts, cups_option_t **options)
{
	int count(0);
	for (QMap<QString,QString>::ConstIterator it=opts.begin(); it!=opts.end(); ++it)
	{
		// only encode those options that doesn't start with "kde-" or "app-".
		if (!it.key().startsWith("kde-") && !it.key().startsWith("app-"))
			count = cupsAddOption(it.key().latin1(),(it.data().isEmpty() ? "" : it.data().latin1()),count,options);
	}
	return count;
}

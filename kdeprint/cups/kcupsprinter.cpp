#include "kcupsprinter.h"
#include "kcupsdialogimpl.h"

#include <qpaintdevicemetrics.h>
#include <klocale.h>

#include <stdlib.h>
#include <cups/cups.h>
#include <cups/ppd.h>

void optionSetToCupsOptions(const OptionSet& opts, int& num_options, cups_option_t **options);
int stringListToCharP(const QStringList& list, char ***clist);
void freeCharP(char **clist, int n);

//**********************************************************************************************

KCupsPrinter::KCupsPrinter()
{
	impl_ = new KCupsDialogImpl(this);
}

KCupsPrinter::~KCupsPrinter()
{
	if (impl_) delete impl_;
}

bool KCupsPrinter::supportPrinterOptions() const
{
	return true;
}

QSize KCupsPrinter::margins() const
{
	// if output to file -> use the metric/margins of Wrapper object
	if (outputToFile()) return KPrinter::margins();

	// else use internal values (pagesize_/margins_)
	if (wrapper_->orientation() == QPrinter::Landscape)
		return QSize(margins_.height(),margins_.width());
	else return margins_;
}

bool KCupsPrinter::printFiles(const QStringList& files)
{
	KPrinter::printFiles(files);

        int	num_options(0);
	cups_option_t	*options(NULL);
	char	**cfiles;
	int	n(0), job_ID(0);

	optionSetToCupsOptions(qtoptions_,num_options,&options);
	n = stringListToCharP(files,&cfiles);
	job_ID = cupsPrintFiles(printerName().local8Bit(),n,(const char**)(cfiles),(docName().isEmpty() ? i18n("KDE Print System") : docName()),num_options,options);
	freeCharP(cfiles,n);

	return (job_ID != 0);
}

void KCupsPrinter::translateOptions(const OptionSet& opts)
{
	// Translate orientation (the wrapper will be set in KPrinter::translateOptions())
	QString	o = opts["orientation-requested"];
	setOption("qt-orientation",(o == "4" || o == "5" ? "Landscape" : "Portrait"));

	// translate copies number
	if (!opts["qt-copies"].isEmpty()) setOption("copies",opts["qt-copies"]);

	// page ranges are handled by CUPS, so application should print all pages
	if (pageSelection() == SystemSide)
	{ // Qt => CUPS
		// translations
		if (!opts["qt-range"].isEmpty())
			setOption("page-ranges",opts["qt-range"]);
		if (!opts["qt-pageorder"].isEmpty()) setOption("OutputOrder",opts["qt-pageorder"]);
		if (opts["qt-pageset"] != "0")
			setOption("page-set",(opts["qt-pageset"] == "1" ? "odd" : "even"));
		setOption("multiple-document-handling",(opts["qt-collate"] == "Collate" ? "separate-documents-collated-copies" : "separate-documents-uncollated-copies"));
		// No clean-up as these values are used if the dialog is re-opened
		// Anyway the application should not use those values (page selection
		// is SystemSide).
		/*qtoptions_.remove("qt-pageset");
		qtoptions_.remove("qt-from");
		qtoptions_.remove("qt-to");
		qtoptions_.remove("qt-range");
		qtoptions_.remove("qt-pageorder");*/
	}
	else
	{ // No translation needed (but range => (from,to))
		QString range = opts["qt-range"];
		if (!range.isEmpty())
		{
			QSize	s = rangeToSize(range);
			setOption("qt-from",QString::number(s.width()));
			setOption("qt-to",QString::number(s.height()));
		}
		// just to be sure, however it shouldn't be necessary !
		qtoptions_.remove("page-ranges");
		qtoptions_.remove("OutputOrder");
		qtoptions_.remove("page-set");
		qtoptions_.remove("multiple-document-handling");
	}

	// Page size -> use sizes from PPD file
	pagesize_ = QSize(-1,-1);
	margins_ = QSize(-1,-1);
	if (!outputToFile())
	{ // output to real printer, try to retrieve page size and margins
	  // from PPD file (usiong specified value if any, otherwise default
	  // PPD value.
		if (!printername_.isEmpty())
		{
			QString	str = cupsGetPPD(printername_.local8Bit());
			if (!str.isEmpty())
			{
				ppd_file_t	*ppd = ppdOpenFile(str.latin1());
				if (ppd)
				{
					QString	psize = opts["PageSize"];
					if (psize.isEmpty())
					{
						ppdMarkDefaults(ppd);
						ppd_choice_t	*ch = ppdFindMarkedChoice(ppd,"PageSize");
						if (ch) psize = QString::fromLatin1(ch->choice);
					}
					if (!psize.isEmpty())
					{
						setOption("qt-pagesize",QString::number((int)(pageNameToPageSize(psize))));
						ppd_size_t	*sz = ppdPageSize(ppd,psize.latin1());
						if (sz)
						{
							pagesize_ = QSize((int)(sz->width),(int)(sz->length));
							margins_ = QSize((int)(sz->left),(int)(sz->bottom));
debug("Page size = (%d,%d)",pagesize_.width(),pagesize_.height());
debug("Page margins = (%d,%d)",margins_.width(),margins_.height());
						}
					}
					ppdClose(ppd);
				}
				::unlink(str.latin1());
			}
		}
		// default values (A4)
		if (!pagesize_.isValid()) pagesize_ = QSize(595,842);
		if (!margins_.isValid()) margins_ = QSize(36,22);
	}

	// call base class function (necessary !!!)
	KPrinter::translateOptions(opts);
}

int KCupsPrinter::metric(int f) const
{
	// if output to file -> use metric/margins from Wrapper object
	if (outputToFile()) return KPrinter::metric(f);

	// else use internal metric (from pagesize_ and margins_)
	bool	land = (wrapper_->orientation() == QPrinter::Landscape);
	int	val;

	switch (f)
	{
		case QPaintDeviceMetrics::PdmWidth:
			val = (land ? pagesize_.height() : pagesize_.width());
			if (!wrapper_->fullPage())
				val -= 2*margins().width();
			break;
		case QPaintDeviceMetrics::PdmHeight:
			val = (land ? pagesize_.width() : pagesize_.height());
			if (!wrapper_->fullPage())
				val -= 2*margins().height();
			break;
		case QPaintDeviceMetrics::PdmWidthMM:
			val = metric(QPaintDeviceMetrics::PdmWidth);
			val = (val*254+360)/720;
			break;
		case QPaintDeviceMetrics::PdmHeightMM:
			val = metric(QPaintDeviceMetrics::PdmHeight);
			val = (val*254+360)/720;
			break;
		default:
			val = KPrinter::metric(f);
			break;
	}
	return val;
}

//**********************************************************************************************

void optionSetToCupsOptions(const OptionSet& opts, int& num_options, cups_option_t **options)
{
	for (QMap<QString,QString>::ConstIterator it=opts.begin();it!=opts.end();++it)
		// skip "qt-..." options as they don't have any meaning for CUPS
		// printing system.
		if (it.key().left(3) != "qt-")
			num_options = cupsAddOption(it.key().latin1(),it.data().latin1(),num_options,options);
}

int stringListToCharP(const QStringList& list, char ***clist)
{
	*clist = (char**)malloc(sizeof(char*)*list.count());
	int	i(0);
	for (QStringList::ConstIterator it=list.begin(); it!=list.end(); ++it, i++)
		(*clist)[i] = strdup((*it).latin1());
	return (int)(list.count());
}

void freeCharP(char **clist, int n)
{
	for (int i=0;i<n;i++)
		free(clist[i]);
	free(clist);
}

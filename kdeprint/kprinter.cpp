#include "kprinter.h"
#include "kdialogimpl.h"
#include "kprintdialog.h"
#include "previewshell.h"
#include "kjoblister.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_CUPS
#include "kcupsprinter.h"
#include "kcupsjoblister.h"
#endif
#include "klpdprinter.h"
#include "kpdqprinter.h"
#include "kpdqjoblister.h"

#include <kapp.h>
#include <kstddirs.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <qmessagebox.h>
#include <klocale.h>
#include <krun.h>

#include <unistd.h>

//**************************************************************************************
// KPrinterWrapper class
//**************************************************************************************

KPrinterWrapper::KPrinterWrapper(KPrinter *prt)
: QPrinter(), printer_(prt)
{
}

KPrinterWrapper::~KPrinterWrapper()
{
}

bool KPrinterWrapper::cmd(int c, QPainter *painter, QPDevCmdParam *p)
{
	return QPrinter::cmd(c,painter,p);
}

int KPrinterWrapper::metric(int m) const
{
	return printer_->metric(m);
}

int KPrinterWrapper::qprinterMetric(int m) const
{
	return QPrinter::metric(m);
}

//**************************************************************************************
// KPrinter class
//**************************************************************************************

KPrinter::KPrinter()
: QPaintDevice(QInternal::Printer|QInternal::ExternalDevice)
{
	// temporary PS file
	QString	fname = QString("kdeprint_%1").arg(getpid());
	tmpbuffer_ = psbuffer_ = locateLocal("tmp",fname);

	// initialize QPrinter wrapper
	wrapper_ = new KPrinterWrapper(this);
	wrapper_->setOutputToFile(true);
	wrapper_->setOutputFileName(psbuffer_);

	// other initialization
	impl_ = 0;
	outputtofile_ = false;
	pageselection_ = KPrinter::SystemSide;
	apptype_ = KPrinter::Dialog;
}

KPrinter::~KPrinter()
{
	// remove temporary file
qDebug("removing tmpfile: %s",tmpbuffer_.latin1());
	::unlink(tmpbuffer_.latin1());

	// delete Wrapper object
	delete wrapper_;
}

bool KPrinter::setup(QWidget *parent)
{
	if (impl_)
	{
		if (KPrintDialog::printerSetup(this, parent))
		{
			preparePrinting();
			return true;
		}
		return false;
	}
	qWarning("No implementation defined !!!");
	return false;
}

void KPrinter::addDialogFlags(int f)
{
	if (impl_) impl_->setDialogFlags(impl_->dialogFlags()|f);
}

void KPrinter::removeDialogFlags(int f)
{
	if (impl_) impl_->setDialogFlags(impl_->dialogFlags()&(~f));
}

void KPrinter::addStandardPage(int p)
{
	if (impl_) impl_->setStandardPages(impl_->standardPages()|p);
}

void KPrinter::removeStandardPage(int p)
{
	if (impl_) impl_->setStandardPages(impl_->standardPages()&(~p));
}

void KPrinter::addDialogPage(KPrintDialogPage *page)
{
	if (impl_) impl_->addDialogPage(page);
}

int KPrinter::copyFlags() const
{
	return (KDialogImpl::Normal | (pageselection_ == KPrinter::ApplicationSide ? KDialogImpl::Current : 0));
}

bool KPrinter::cmd(int c, QPainter *painter, QPDevCmdParam *p)
{
	bool value(true);
	value = wrapper_->cmd(c,painter,p);
	if (c == QPaintDevice::PdcEnd && !outputtofile_)
	{
		if (option("qt-preview") != "1" || PreviewShell::preview(psbuffer_))
			value = value && printFile(psbuffer_);
	}
	return value;
}

void KPrinter::translateOptions(const OptionSet& opts)
{
	wrapper_->setCreator(opts["qt-creator"]);
	wrapper_->setDocName(opts["qt-docname"]);
	wrapper_->setFullPage((opts["qt-fullpage"] == "1"));
	wrapper_->setColorMode((opts["qt-colormode"] == "GrayScale" ? QPrinter::GrayScale : QPrinter::Color));
	wrapper_->setOrientation((opts["qt-orientation"] == "Landscape" ? QPrinter::Landscape : QPrinter::Portrait));
	wrapper_->setPageSize((opts["qt-pagesize"].isEmpty() ? QPrinter::A4 : (QPrinter::PageSize)(opts["qt-pagesize"].toInt())));
}

bool KPrinter::printFile(const QString& filename)
{
	QStringList	files(filename);
	return printFiles(files);
}

bool KPrinter::printFiles(const QStringList&)
{
	QString	cmd = QString("kprintjob -d %1").arg(printerName());
	KRun::runCommand(cmd);
	return false;
}

//**************************************************************************************
// QPrinter interface
//**************************************************************************************

bool KPrinter::outputToFile() const
{ return outputtofile_; }

void KPrinter::setOutputToFile(bool on)
{
	outputtofile_ = on;
	if (!outputtofile_)
	{
		// reset PS buffer to something "temporary"
		psbuffer_ = tmpbuffer_;
		wrapper_->setOutputFileName(psbuffer_);
	}
}

QString KPrinter::outputFileName() const
{ return psbuffer_; }

void KPrinter::setOutputFileName(const QString& f)
{
	psbuffer_ = f;
	wrapper_->setOutputFileName(psbuffer_);
}

bool KPrinter::newPage()
{ return wrapper_->newPage(); }

int KPrinter::numCopies() const
{
	bool	ok;
	int	p = option("qt-copies").toInt(&ok);
	return (ok ? p : 1);
}

bool KPrinter::abort()
{ return wrapper_->abort(); }

bool KPrinter::aborted() const
{ return wrapper_->aborted(); }

QSize KPrinter::margins() const
{
	return wrapper_->margins();
}

int KPrinter::metric(int m) const
{
	return wrapper_->qprinterMetric(m);
}

void KPrinter::setOrientation(Orientation o)
{
	setOption("qt-orientation",(o == Landscape ? "Landscape" : "Portrait"));
	setOption("orientation-requested",(o == Landscape ? "4" : "3"));
	if (impl_ && supportPrinterOptions()) impl_->setGlobalOption("orientation-requested",(o == Landscape ? "4" : "3"));
}

void KPrinter::setPageSize(PageSize s)
{
	setOption("qt-pagesize",QString::number((int)s));
	if (impl_ && supportPrinterOptions()) impl_->setGlobalOption("PageSize",pageSizeToPageName(s));
}

void KPrinter::setOptions(const OptionSet& opts)
{ // This functions remove all options except those with "qt-..."
  // which correspond to externally-sets options (use teh value
  // from "opts" if specified
	OptionSet	tmpset = qtoptions_;
	qtoptions_ = opts;
	for (QMap<QString,QString>::ConstIterator it=tmpset.begin();it!=tmpset.end();++it)
		if (it.key().left(3) == "qt-" && !(qtoptions_.contains(it.key())))
			qtoptions_[it.key()] = it.data();
}

bool KPrinter::supportPrinterOptions() const
{
	return false;
}

void KPrinter::initOptions(const OptionSet& opts)
{ // This function can be used to initialize the KPrinter object just after
  // creation to set some options. Non global options will be propagated to
  // all listed printers (non-global => start with "qt-...")
	qtoptions_ = opts;
	if (!impl_ || !supportPrinterOptions()) return;
	for (OptionSet::ConstIterator it=opts.begin(); it!=opts.end(); ++it)
		impl_->setGlobalOption(it.key(),it.data());
}

//**************************************************************************************
// KPrinterFactory class
//**************************************************************************************

KPrinterFactory* KPrinterFactory::unique_ = 0;

static void cleanup_kdeprint()
{
	KPrinterFactory::release();
}

KPrinterFactory* KPrinterFactory::self()
{
	if (!unique_)
	{
		unique_ = new KPrinterFactory();
		qAddPostRoutine(cleanup_kdeprint);
	}
	return unique_;
}

void KPrinterFactory::release()
{
	if (unique_)
		delete unique_;
}

KPrinterFactory::KPrinterFactory()
{
	if (unique_)
		qFatal("More than 1 KPrinterFactory object in the same app !");
	printer_ = 0;
	joblister_ = 0;

	// default to LPD
	type_ = LPD;

	// load configuration
	loadConfig();
	KGlobal::iconLoader()->addAppDir("kdeprint");
}

KPrinterFactory::~KPrinterFactory()
{
	if (printer_) delete printer_;
}

KPrinter* KPrinterFactory::printer()
{
	if (!printer_)
		createPrinter();
	return printer_;
}

KJobLister* KPrinterFactory::jobLister()
{
	if (!joblister_)
		createLister();
	return joblister_;
}

void KPrinterFactory::createPrinter()
{
	if (printer_)
	{
		delete printer_;
		printer_ = 0;
	}

	switch (type_)
	{
		case LPD:
			printer_ = new KLpdPrinter();
			break;
		case CUPS:
#ifdef HAVE_CUPS
			printer_ = new KCupsPrinter();
#else
			QMessageBox::critical(kapp->mainWidget(),i18n("Print configuration error"),i18n("<p>You system is configured to use the CUPS print system. However, the KDE environment is not compiled with CUPS support enabled. You should recompile the KDE print support or contact your system administrator. The current application will now try to use the old LPD print system.</p>"),QMessageBox::Ok|QMessageBox::Default,0);
			printer_ = new KLpdPrinter();
#endif
			break;
		case PDQ:
			printer_ = new KPdqPrinter();
			break;
		default:
			qFatal("Printer creation: Unknown print system !");
			break;
	}
}

void KPrinterFactory::createLister()
{
	if (joblister_)
	{
		delete joblister_;
		joblister_ = 0;
	}

	switch (type_)
	{
		case CUPS:
#ifdef HAVE_CUPS
			joblister_ = new KCupsJobLister();
#else
			QMessageBox::critical(kapp->mainWidget(),i18n("Print configuration error"),i18n("<p>You system is configured to use the CUPS print system. However, the KDE environment is not compiled with CUPS support enabled. You should recompile the KDE print support or contact your system administrator. The current application will now try to use the old LPD print system.</p>"),QMessageBox::Ok|QMessageBox::Default,0);
			joblister_ = new KJobLister();
#endif
			break;
		case PDQ:
			joblister_ = new KPdqJobLister();
			break;
		default:
			qWarning("Job lister creation: Unknown print systen !");
			joblister_ = 0;
			break;
	}
}

void KPrinterFactory::loadConfig()
{
	KConfig	config_("kdeprintrc");
	config_.setGroup("General");
	type_ = (PrintSystemType)(config_.readNumEntry("PrintSystem",LPD));	// default to LPD (=0)
}

bool KPrinterFactory::setPrintSystem(PrintSystemType pst)
{
	type_ = pst;
	createPrinter();
	return true;
}

//**************************************************************************************
// Util functions
//**************************************************************************************

KPrinter::PageSize pageNameToPageSize(const QString& name)
{
	if (name == "Letter") return KPrinter::Letter;
	else if (name == "Legal") return KPrinter::Legal;
	else if (name == "A4") return KPrinter::A4;
	else if (name == "A3") return KPrinter::A3;
	else if (name == "Executive") return KPrinter::Executive;
	else if (name == "Ledger") return KPrinter::Ledger;
	else if (name == "Tabloid") return KPrinter::Tabloid;
	else if (name == "Folio") return KPrinter::Folio;
	else if (name == "A5") return KPrinter::A5;
	else if (name == "A6") return KPrinter::A6;
	else if (name == "A7") return KPrinter::A7;
	else if (name == "A8") return KPrinter::A8;
	else if (name == "A9") return KPrinter::A9;
	else if (name == "A2") return KPrinter::A2;
	else if (name == "A1") return KPrinter::A1;
	else if (name == "A0") return KPrinter::A0;
	else if (name == "B0" || name == "B0ISO") return KPrinter::B0;
	else if (name == "B1" || name == "B1ISO") return KPrinter::B1;
	else if (name == "B2" || name == "B2ISO") return KPrinter::B2;
	else if (name == "B3" || name == "B3ISO") return KPrinter::B3;
	else if (name == "B4" || name == "B4ISO") return KPrinter::B4;
	else if (name == "B5" || name == "B5ISO") return KPrinter::B5;
	else if (name == "B6" || name == "B6ISO") return KPrinter::B6;
	else if (name == "B7" || name == "B7ISO") return KPrinter::B7;
	else if (name == "B8" || name == "B8ISO") return KPrinter::B8;
	else if (name == "B9" || name == "B9ISO") return KPrinter::B9;
	else if (name == "C5" || name == "C5E" || name == "EnvC5") return KPrinter::C5E;
	else if (name == "DL" || name == "DLE" || name == "EnvDL") return KPrinter::DLE;
	else if (name == "Comm10" || name == "COM10" || name == "Env10") return KPrinter::Comm10E;
	else return KPrinter::A4;
}

const char* pageSizeToPageName(KPrinter::PageSize s)
{
	switch(s)
	{
		case KPrinter::Letter: return "Letter";
		case KPrinter::Legal: return "Legal";
		case KPrinter::A4: return "A4";
		case KPrinter::A3: return "A3";
		case KPrinter::Executive: return "Executive";
		case KPrinter::Ledger: return "Ledger";
		case KPrinter::Tabloid: return "Tabloid";
		case KPrinter::Folio: return "Folio";
		case KPrinter::A5: return "A5";
		case KPrinter::A6: return "A6";
		case KPrinter::A7: return "A7";
		case KPrinter::A8: return "A8";
		case KPrinter::A9: return "A9";
		case KPrinter::A2: return "A2";
		case KPrinter::A1: return "A1";
		case KPrinter::A0: return "A0";
		case KPrinter::B0: return "B0";
		case KPrinter::B1: return "B1";
		case KPrinter::B2: return "B2";
		case KPrinter::B3: return "B3";
		case KPrinter::B4: return "B4";
		case KPrinter::B5: return "B5";
		case KPrinter::B6: return "B6";
		case KPrinter::B7: return "B7";
		case KPrinter::B8: return "B8";
		case KPrinter::B9: return "B9";
		case KPrinter::C5E: return "C5";
		case KPrinter::DLE: return "DL";
		case KPrinter::Comm10E: return "Comm10";
		default: return "A4";
	}
}

QSize rangeToSize(const QString& s)
{
	QString	range = s;
	int	p(-1);
	int	from, to;

	if ((p=range.find(',')) != -1)
		range.truncate(p);
	if ((p=range.find('-')) != -1)
	{
		from = range.left(p).toInt();
		to = range.right(range.length()-p-1).toInt();
	}
	else if (!range.isEmpty())
		from = to = range.toInt();
	else
		from = to = 0;

	return QSize(from,to);
}

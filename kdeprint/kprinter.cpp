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

#include "kprinter.h"
#include "kprinterimpl.h"
#include "kprintdialog.h"
#include "kprintpreview.h"
#include "kmfactory.h"
#include "kmuimanager.h"

#include <qpaintdevicemetrics.h>
#include <qfile.h>
#include <qtl.h>
#include <kapp.h>
#include <kstddirs.h>
#include <kglobal.h>
#include <kconfig.h>
#include <krun.h>
#include <knotifyclient.h>
#include <kdebug.h>
#include <klocale.h>
#include <kprocess.h>
#include <klibloader.h>
#include <kmessagebox.h>

void dumpOptions(const QMap<QString,QString>& opts);
void reportError(KPrinter*);

//**************************************************************************************
// KPrinterWrapper class
//**************************************************************************************

KPrinterWrapper::KPrinterWrapper(KPrinter *prt, QPrinter::PrinterMode m)
: QPrinter(m), m_printer(prt)
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
	return m_printer->metric(m);
}

int KPrinterWrapper::qprinterMetric(int m) const
{
	return QPrinter::metric(m);
}

//**************************************************************************************
// KPrinterPrivate class
//**************************************************************************************

class KPrinterPrivate
{
public:
	KPrinterImpl	*m_impl;
	bool		m_restore;
	bool		m_previewonly;
};

//**************************************************************************************
// KPrinter class
//**************************************************************************************

KPrinter::KPrinter(bool restore, QPrinter::PrinterMode m)
: QPaintDevice(QInternal::Printer|QInternal::ExternalDevice)
{
	init(restore, m);
}

KPrinter::~KPrinter()
{
	// delete Wrapper object
	delete m_wrapper;

	// save current options
	if (d->m_restore)
		saveSettings();

	// delete private data
	delete d;
}

void KPrinter::init(bool restore, QPrinter::PrinterMode m)
{
	// initialize QPrinter wrapper
	m_wrapper = new KPrinterWrapper(this, m);

	// Private data initialization
	d = new KPrinterPrivate;
	d->m_impl = KMFactory::self()->printerImplementation();
	d->m_restore = restore;
	d->m_previewonly = false;

	// other initialization
	m_tmpbuffer = d->m_impl->tempFile();
	m_ready = false;

	// reload options from implementation (static object)
	if (d->m_restore)
		loadSettings();
}

void KPrinter::loadSettings()
{
	m_options = d->m_impl->loadOptions();

	// load latest used printer from config file
	KConfig	*conf = KGlobal::config();
	conf->setGroup("KPrinter Settings");
	setSearchName(conf->readEntry("Printer",QString::null));
	// latest used print command
	setOption("kde-printcommand",conf->readEntry("PrintCommand"));
}

void KPrinter::saveSettings()
{
	d->m_impl->saveOptions(m_options);

	// save latest used printer to config file
	KConfig	*conf = KGlobal::config();
	conf->setGroup("KPrinter Settings");
	conf->writeEntry("Printer",searchName());
	// latest used print command
	conf->writeEntry("PrintCommand",option("kde-printcommand"));
}

bool KPrinter::setup(QWidget *parent, const QString& caption)
{
	bool	state = KPrintDialog::printerSetup(this, parent, caption);
	return state;
}

void KPrinter::addStandardPage(int p)
{
	KMFactory::self()->settings()->standardDialogPages |= p;
}

void KPrinter::removeStandardPage(int p)
{
	KMFactory::self()->settings()->standardDialogPages &= (~p);
}

void KPrinter::addDialogPage(KPrintDialogPage *page)
{
	KMFactory::self()->uiManager()->addPrintDialogPage(page);
}

void KPrinter::setPageSelection(PageSelectionType t)
{
	KMFactory::self()->settings()->pageSelection = t;
}

KPrinter::PageSelectionType KPrinter::pageSelection()
{
	return (PageSelectionType)KMFactory::self()->settings()->pageSelection;
}

void KPrinter::setApplicationType(ApplicationType t)
{
	KMFactory::self()->settings()->application = t;
}

KPrinter::ApplicationType KPrinter::applicationType()
{
	return (ApplicationType)KMFactory::self()->settings()->application;
}

bool KPrinter::cmd(int c, QPainter *painter, QPDevCmdParam *p)
{
	bool value(true);
	if (c == QPaintDevice::PdcBegin)
		preparePrinting();
	value = m_wrapper->cmd(c,painter,p);
	if (c == QPaintDevice::PdcEnd)
	{
		// this call should take care of everything (preview, output-to-file, filtering, ...)
		value = value && printFiles(QStringList(m_wrapper->outputFileName()),true);
		// reset "ready" state
		finishPrinting();
	}
	return value;
}

void KPrinter::translateQtOptions()
{
	m_wrapper->setCreator(creator());
	m_wrapper->setDocName(docName());
	m_wrapper->setFullPage(fullPage());
	m_wrapper->setColorMode((QPrinter::ColorMode)colorMode());
	m_wrapper->setOrientation((QPrinter::Orientation)orientation());
	m_wrapper->setPageSize((QPrinter::PageSize)pageSize());
	m_wrapper->setOutputToFile(true);
	m_wrapper->setOutputFileName(m_tmpbuffer);
	m_wrapper->setNumCopies(option("kde-qtcopies").isEmpty() ? 1 : option("kde-qtcopies").toInt());
	// for special printers, copies are handled by Qt
	if (option("kde-isspecial") == "1")
		m_wrapper->setNumCopies(numCopies());
}

bool KPrinter::printFiles(const QStringList& l, bool flag)
{
	QStringList	files(l);
	bool		status(true);

	// First apply possible filters, and update "remove" flag if filters has
	// been applied (result == 0, means nothing happened).
	int	fresult = d->m_impl->filterFiles(this, files, flag);
	if (fresult == -1)
	{
		reportError(this);
		status = false;
	}
	else if (fresult == 1)
		flag = true;

	// Automatic conversion to format supported by print system
	fresult = d->m_impl->autoConvertFiles(this, files, flag);
	if (fresult == -1)
	{
		reportError(this);
		status = false;
	}
	else if (fresult == 1)
		flag = true;

	// Continue if status is OK (filtering succeeded) and no output-to-file
	if (status && files.count() > 0)
	{
		// Show preview if needed (only possible for a single file !), and stop
		// if the user requested it. Force preview if preview-only mode has been set: it
		// then use by default the first file in the list.
		if (((files.count() != 1 || option("kde-preview") != "1") && !d->m_previewonly) || KPrintPreview::preview(files[0], d->m_previewonly))
		{
			// check if printing has been prepared (it may be not prepared if the KPrinter object is not
			// use as a QPaintDevice object)
			preparePrinting();

			if (!d->m_impl->printFiles(this, files, flag))
			{
				reportError(this);
				status = false;
			}
			else
			{
				if (!outputToFile())
				{
					QStringList args;
					args << "-d";
					args << printerName();
					kapp->kdeinitExec("kjobviewer", args);
				}
			}
		}
		else if (flag)
		// situation: only one file, it has been previewed and printing has been cancelled, then
		//            we should remove the file ourself
		{
			QFile::remove(files[0]);
		}
	}
	finishPrinting();
	return status;
}

void KPrinter::preparePrinting()
{
	// check if already prepared (-> do nothing)
	if (m_ready) return;

	// re-initialize error
	setErrorMessage(QString::null);

	// re-initialize margins and page size (by default, use Qt mechanism)
	setMargins(QSize(-1,-1));
	setRealPageSize(QSize(-1,-1));

	// print-system-specific setup, only if not printing to file
	if (option("kde-isspecial") != "1")
		d->m_impl->preparePrinting(this);

	// standard Qt settings
	translateQtOptions();

	m_ready = true;
dumpOptions(m_options);
}

void KPrinter::finishPrinting()
{
	m_ready = false;
}

QValueList<int> KPrinter::pageList() const
{
	QValueList<int>	list;
	int	mp(minPage()), MP(maxPage());
	if (mp > 0 && MP > 0 && MP >= mp)
	{ // do something only if bounds specified
		if (option("kde-current") == "1")
		{ // print only current page
			int	pp = currentPage();
			if (pp >= mp && pp <= MP) list.append(pp);
		}
		else
		{
			// process range specification
			if (!option("kde-range").isEmpty())
			{
				QStringList	ranges = QStringList::split(',',option("kde-range"),false);
				// use a temporary map to insure uniqueness of pages.
				QMap<int,int>	pages;
				for (QStringList::ConstIterator it=ranges.begin();it!=ranges.end();++it)
				{
					int	p = (*it).find('-');
					bool	ok;
					if (p == -1)
					{
						int	pp = (*it).toInt(&ok);
						if (ok && pp >= mp && pp <= MP)
							pages[pp] = 1;
					}
					else
					{
						int	p1(0), p2(0);
						p1 = (*it).left(p).toInt(&ok);
						if (ok) p2 = (*it).right((*it).length()-p-1).toInt(&ok);
						if (ok && p1 <= p2)
						{
							// clip to min/max
							p1 = QMAX(mp,p1);
							p2 = QMIN(MP,p2);
							for (int i=p1;i<=p2;i++)
								pages[i] = 1;
						}
					}
				}
				// translate the map into the list
				for (QMap<int,int>::ConstIterator it=pages.begin();it!=pages.end();++it)
					list.append(it.key());
				// sort the list
				qHeapSort(list);
			}
			else
			{ // add all pages between min and max
				for (int i=mp;i<=MP;i++) list.append(i);
			}

			// revert the list if needed
			if (pageOrder() == LastPageFirst)
			{
				for (uint i=0;i<(list.count()/2);i++)
					qSwap(list[i],list[list.count()-1-i]);
			}

			// select page set if needed
			if (pageSet() != AllPages)
			{
				bool	keepEven = (pageSet() == EvenPages);
				for (QValueList<int>::Iterator it=list.begin();it!=list.end();)
					if ((((*it) % 2) != 0 && keepEven) ||
					    (((*it) % 2) == 0 && !keepEven)) it = list.remove(it);
					else ++it;
			}
		}
	}
	return list;
}

//**************************************************************************************
// QPrinter interface
//**************************************************************************************

int KPrinter::numCopies() const
{
	bool	ok;
	int	p = option("kde-copies").toInt(&ok);
	return (ok ? p : 1);
}

QSize KPrinter::margins() const
{
	/*if (m_margins.isValid())
		if (orientation() == KPrinter::Landscape) return QSize(m_margins.height(),m_margins.width());
		else return m_margins;
	else return m_wrapper->margins();*/
	return m_wrapper->margins();
}

int KPrinter::metric(int m) const
{
	if (!m_pagesize.isValid())
		return m_wrapper->qprinterMetric(m);

	int	val(0);
	bool	land = (orientation() == KPrinter::Landscape);
	switch ( m )
	{
		case QPaintDeviceMetrics::PdmWidth:
			val = (land ? m_pagesize.height() : m_pagesize.width());
			if ( !fullPage() ) val -= 2*margins().width();
			break;
		case QPaintDeviceMetrics::PdmHeight:
			val = (land ? m_pagesize.width() : m_pagesize.height());
			if ( !fullPage() ) val -= 2*margins().height();
			break;
		case QPaintDeviceMetrics::PdmWidthMM:
			val = metric( QPaintDeviceMetrics::PdmWidth );
			val = (val * 254 + 360) / 720; // +360 to get the right rounding
			break;
		case QPaintDeviceMetrics::PdmHeightMM:
			val = metric( QPaintDeviceMetrics::PdmHeight );
			val = (val * 254 + 360) / 720;
			break;
		default:
			val = m_wrapper->qprinterMetric(m);
			break;
	}
	return val;
}

void KPrinter::setOrientation(Orientation o)
{
	KMFactory::self()->settings()->orientation = o;
	setOption("kde-orientation",(o == Landscape ? "Landscape" : "Portrait"));
	d->m_impl->broadcastOption("kde-orientation",(o == Landscape ? "Landscape" : "Portrait"));
}

void KPrinter::setPageSize(PageSize s)
{
	KMFactory::self()->settings()->pageSize = s;
	setOption("kde-pagesize",QString::number((int)s));
	d->m_impl->broadcastOption("kde-pagesize",option("kde-pagesize"));
}

void KPrinter::setOptions(const QMap<QString,QString>& opts)
{ // This functions remove all options except those with "kde-..."
  // which correspond to externally-sets options (use the value
  // from "opts" if specified
	QMap<QString,QString>	tmpset = m_options;
	m_options = opts;
	// remove some problematic options that may not be overwritten (ugly hack).
	// Default values will be used instead, except if the dialog has set new ones.
	tmpset.remove("kde-pagesize");
	tmpset.remove("kde-orientation");
	tmpset.remove("kde-colormode");
	for (QMap<QString,QString>::ConstIterator it=tmpset.begin();it!=tmpset.end();++it)
		if (it.key().left(4) == "kde-" && !(m_options.contains(it.key())))
			m_options[it.key()] = it.data();
}

void KPrinter::initOptions(const QMap<QString,QString>& opts)
{ // This function can be used to initialize the KPrinter object just after
  // creation to set some options. Non global options will be propagated to
  // all listed printers (non-global => start with "kde-...")
	m_options = opts;
	for (QMap<QString,QString>::ConstIterator it=opts.begin(); it!=opts.end(); ++it)
		if (it.key().left(4) != "kde-")
			d->m_impl->broadcastOption(it.key(),it.data());
}

void KPrinter::reload()
{
	d->m_impl = KMFactory::self()->printerImplementation();
	int	global = KMFactory::self()->settings()->orientation;
	if (global != -1) setOrientation((KPrinter::Orientation)global);
	global = KMFactory::self()->settings()->pageSize;
	if (global != -1) setPageSize((KPrinter::PageSize)global);
	//initOptions(m_options);
}

//**************************************************************************************
// Util functions
//**************************************************************************************

void reportError(KPrinter *p)
{
	if (!KNotifyClient::event("printerror",i18n("<p><nobr>A print error occured. Error message received from system:</nobr></p><br>%1").arg(p->errorMessage())))
		kdDebug() << "could not send notify event" << endl;
}

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

void dumpOptions(const QMap<QString,QString>& opts)
{
	kdDebug() << "********************" << endl;
	for (QMap<QString,QString>::ConstIterator it=opts.begin(); it!=opts.end(); ++it)
		kdDebug() << it.key() << " = " << it.data() << endl;
}

KPrinterImpl* KPrinter::implementation() const
{ return d->m_impl; }

const QString& KPrinter::option(const QString& key) const
{ return m_options[key]; }

void KPrinter::setOption(const QString& key, const QString& value)
{ m_options[key] = value; }

QString KPrinter::docName() const
{ return option("kde-docname"); }

void KPrinter::setDocName(const QString& d)
{ setOption("kde-docname",d); }

QString KPrinter::creator() const
{ return option("kde-creator"); }

void KPrinter::setCreator(const QString& d)
{ setOption("kde-creator",d); }

bool KPrinter::fullPage() const
{ return (option("kde-fullpage") == "1"); }

void KPrinter::setFullPage(bool on)
{ setOption("kde-fullpage",(on ? "1" : "0")); }

KPrinter::ColorMode KPrinter::colorMode() const
{ return (KPrinter::ColorMode)(option("kde-colormode") == "GrayScale" ? GrayScale : Color); }

void KPrinter::setColorMode(ColorMode m)
{ setOption("kde-colormode",(m == Color ? "Color" : "GrayScale")); }

void KPrinter::setNumCopies(int n)
{ setOption("kde-copies",QString::number(n)); }

KPrinter::Orientation KPrinter::orientation() const
{ return (option("kde-orientation") == "Landscape" ? Landscape : Portrait); }

KPrinter::PageOrder KPrinter::pageOrder() const
{ return (option("kde-pageorder") == "Reverse" ? LastPageFirst : FirstPageFirst); }

void KPrinter::setPageOrder(PageOrder o)
{ setOption("kde-pageorder",(o == LastPageFirst ? "Reverse" : "Forward")); }

KPrinter::CollateType KPrinter::collate() const
{ return (option("kde-collate") == "Collate" ? Collate : Uncollate); }

void KPrinter::setCollate(CollateType c)
{ setOption("kde-collate",(c == Collate ? "Collate" : "Uncollate")); }

int KPrinter::minPage() const
{ return (option("kde-minpage").isEmpty() ? 0 : option("kde-minpage").toInt()); }

int KPrinter::maxPage() const
{ return (option("kde-maxpage").isEmpty() ? 0 : option("kde-maxpage").toInt()); }

void KPrinter::setMinMax(int m, int M)
{ setOption("kde-minpage",QString::number(m)); setOption("kde-maxpage",QString::number(M)); }

int KPrinter::fromPage() const
{ return (option("kde-frompage").isEmpty() ? 0 : option("kde-frompage").toInt()); }

int KPrinter::toPage() const
{ return (option("kde-topage").isEmpty() ? 0 : option("kde-topage").toInt()); }

void KPrinter::setFromTo(int m, int M)
{ setOption("kde-frompage",QString::number(m)); setOption("kde-topage",QString::number(M)); setOption("kde-range",(m>0 && M>0 ? QString("%1-%2").arg(m).arg(M) : QString::fromLatin1(""))); }

KPrinter::PageSize KPrinter::pageSize() const
{ return (option("kde-pagesize").isEmpty() ? A4 : (PageSize)option("kde-pagesize").toInt()); }

KPrinter::PageSetType KPrinter::pageSet() const
{ return (option("kde-pageset").isEmpty() ? AllPages : (PageSetType)(option("kde-pageset").toInt())); }

int KPrinter::currentPage() const
{ return (option("kde-currentpage").isEmpty() ? 0 : option("kde-currentpage").toInt()); }

void KPrinter::setCurrentPage(int p)
{ setOption("kde-currentpage",QString::number(p)); }

QString KPrinter::printerName() const
{ return m_printername; }

void KPrinter::setPrinterName(const QString& s)
{ m_printername = s; }

QString KPrinter::printProgram() const
{ return (option("kde-isspecial") == "1" ? option("kde-special-command") : QString::null); }

void KPrinter::setPrintProgram(const QString& prg)
{
	if (prg.isNull())
	{
		setOption("kde-isspecial", "0");
		m_options.remove("kde-special-command");
	}
	else
	{
		setOption("kde-isspecial", "1");
		QString	s(prg);
		if (s.find("%in") == -1)
			s.append(" %in");
		setOption("kde-special-command", s);
	}
}

QString KPrinter::printerSelectionOption() const
{ return QString::fromLatin1(""); }

void KPrinter::setPrinterSelectionOption(const QString&)
{}

const QMap<QString,QString>& KPrinter::options() const
{ return m_options; }

QString KPrinter::searchName() const
{ return m_searchname; }

void KPrinter::setSearchName(const QString& s)
{ m_searchname = s; }

bool KPrinter::newPage()
{ return m_wrapper->newPage(); }

QString KPrinter::outputFileName() const
{ return option("kde-outputfilename"); }

void KPrinter::setOutputFileName(const QString& f)
{ setOption("kde-outputfilename",f); setOutputToFile(!f.isEmpty()); }

bool KPrinter::outputToFile() const
{ return (option("kde-outputtofile") == "1" || (option("kde-isspecial") == "1" && option("kde-special-command").isEmpty())); }

void KPrinter::setOutputToFile(bool on)
{
	setOption("kde-outputtofile",(on ? "1" : "0"));
	if (on)
	{
		setOption("kde-special-command",QString::null);
		setOption("kde-isspecial","1");
	}
}

bool KPrinter::abort()
{ return m_wrapper->abort(); }

bool KPrinter::aborted() const
{ return m_wrapper->aborted(); }

void KPrinter::setMargins(QSize m)
{ m_margins = m; }

QSize KPrinter::realPageSize() const
{ return m_pagesize; }

void KPrinter::setRealPageSize(QSize p)
{ m_pagesize = p; }

QString KPrinter::errorMessage() const
{ return m_errormsg; }

void KPrinter::setErrorMessage(const QString& msg)
{ m_errormsg = msg; }

/* we're using a builtin member to store this state because we don't
 * want to keep it from object to object. So there's no need to use
 * the QMap structure to store this
 */
void KPrinter::setPreviewOnly(bool on)
{ d->m_previewonly = on; }

bool KPrinter::previewOnly() const
{ return d->m_previewonly; }

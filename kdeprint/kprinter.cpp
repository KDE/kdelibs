/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  $Id:  $
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
#include <kapp.h>
#include <kstddirs.h>
#include <kglobal.h>
#include <kconfig.h>
#include <krun.h>

void dumpOptions(const QMap<QString,QString>& opts);

//**************************************************************************************
// KPrinterWrapper class
//**************************************************************************************

KPrinterWrapper::KPrinterWrapper(KPrinter *prt)
: QPrinter(), m_printer(prt)
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
// KPrinter class
//**************************************************************************************

KPrinter::KPrinter()
: QPaintDevice(QInternal::Printer|QInternal::ExternalDevice)
{
	// temporary PS file
	QString	fname = locateLocal("tmp","kdeprint_");
	fname.append(KApplication::randomString(8));
	m_tmpbuffer = m_psbuffer = fname;

	// initialize QPrinter wrapper
	m_wrapper = new KPrinterWrapper(this);
	m_wrapper->setOutputToFile(true);
	m_wrapper->setOutputFileName(m_psbuffer);

	// other initialization
	m_impl = KMFactory::self()->printerImplementation();
	m_outputtofile = false;

	// reload options from implementation (static object)
	loadSettings();
}

KPrinter::~KPrinter()
{
	// remove temporary file
	QFile::remove(m_tmpbuffer);

	// delete Wrapper object
	delete m_wrapper;

	// save current options
	saveSettings();
}

void KPrinter::loadSettings()
{
	if (m_impl) m_options = m_impl->loadOptions();
	setOutputFileName(option("kde-outputfilename"));
	setOutputToFile((option("kde-outputtofile") == "1"));

	// load latest used printer from config file
	KConfig	*conf = KGlobal::config();
	conf->setGroup("KPrinter Settings");
	setSearchName(conf->readEntry("Printer",QString::null));
}

void KPrinter::saveSettings()
{
	m_options["kde-outputfilename"] = outputFileName();
	m_options["kde-outputtofile"] = (outputToFile() ? "1" : "0");
	if (m_impl) m_impl->saveOptions(m_options);

	// save latest used printer to config file
	KConfig	*conf = KGlobal::config();
	conf->setGroup("KPrinter Settings");
	conf->writeEntry("Printer",searchName());
}

bool KPrinter::setup(QWidget *parent)
{
	if (m_impl)
	{
		if (KPrintDialog::printerSetup(this, parent))
		{
			preparePrinting();
dumpOptions(m_options);
			return true;
		}
		return false;
	}
	qWarning("No implementation defined !!!");
	return false;
}

void KPrinter::addStandardPage(int p)
{
	KMFactory::self()->uiManager()->addPrintDialogStandardPage(p);
}

void KPrinter::removeStandardPage(int p)
{
	KMFactory::self()->uiManager()->removePrintDialogStandardPage(p);
}

void KPrinter::addDialogPage(KPrintDialogPage *page)
{
	KMFactory::self()->uiManager()->addPrintDialogPage(page);
}

void KPrinter::setPageSelection(PageSelectionType t)
{
	KMFactory::self()->uiManager()->setPageSelection(t);
}

KPrinter::PageSelectionType KPrinter::pageSelection()
{
	return (PageSelectionType)KMFactory::self()->uiManager()->pageSelection();
}

void KPrinter::setApplicationType(ApplicationType t)
{
	KMFactory::self()->uiManager()->setApplicationType(t);
}

KPrinter::ApplicationType KPrinter::applicationType()
{
	return (ApplicationType)KMFactory::self()->uiManager()->applicationType();
}

bool KPrinter::cmd(int c, QPainter *painter, QPDevCmdParam *p)
{
	bool value(true);
	value = m_wrapper->cmd(c,painter,p);
	if (c == QPaintDevice::PdcEnd && !m_outputtofile)
	{
		if (option("kde-preview") != "1" || KPrintPreview::preview(m_psbuffer))
			value = value && printFiles(QStringList(m_psbuffer));
	}
	return value;
}

void KPrinter::translateQtOptions()
{
	m_wrapper->setCreator(option("kde-creator"));
	m_wrapper->setDocName(option("kde-docname"));
	m_wrapper->setFullPage((option("kde-fullpage") == "1"));
	m_wrapper->setColorMode((option("kde-colormode") == "GrayScale" ? QPrinter::GrayScale : QPrinter::Color));
	m_wrapper->setOrientation((option("kde-orientation") == "Landscape" ? QPrinter::Landscape : QPrinter::Portrait));
	m_wrapper->setPageSize((option("kde-pagesize").isEmpty() ? QPrinter::A4 : (QPrinter::PageSize)(option("kde-pagesize").toInt())));
}

bool KPrinter::printFiles(const QStringList& l)
{
	QString	cmd = QString("kjobviewer -d %1").arg(printerName());
	KRun::runCommand(cmd);
	return m_impl->printFiles(this, l);
}

void KPrinter::preparePrinting()
{
	// re-initialize margins and page size (by default, use Qt mechanism)
	setMargins(QSize(-1,-1));
	setRealPageSize(QSize(-1,-1));

	// print-system-specific setup, only if not printing to file
	if (m_impl && !outputToFile())
		m_impl->preparePrinting(this);

	// standard Qt settings
	translateQtOptions();
}

//**************************************************************************************
// QPrinter interface
//**************************************************************************************

void KPrinter::setOutputToFile(bool on)
{
	m_outputtofile = on;
	if (!m_outputtofile)
	{
		// reset PS buffer to something "temporary"
		m_psbuffer = m_tmpbuffer;
		m_wrapper->setOutputFileName(m_psbuffer);
	}
}

void KPrinter::setOutputFileName(const QString& f)
{
	m_psbuffer = f;
	m_wrapper->setOutputFileName(m_psbuffer);
}

int KPrinter::numCopies() const
{
	bool	ok;
	int	p = option("kde-copies").toInt(&ok);
	return (ok ? p : 1);
}

QSize KPrinter::margins() const
{
	if (m_margins.isValid())
		if (orientation() == KPrinter::Landscape) return QSize(m_margins.height(),m_margins.width());
		else return m_margins;
	else return m_wrapper->margins();
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
	setOption("kde-orientation",(o == Landscape ? "Landscape" : "Portrait"));
	if (m_impl) m_impl->broadcastOption("kde-orientation",(o == Landscape ? "Landscape" : "Portrait"));
}

void KPrinter::setPageSize(PageSize s)
{
	setOption("kde-pagesize",QString::number((int)s));
	if (m_impl) m_impl->broadcastOption("kde-pagesize",option("kde-pagesize"));
}

void KPrinter::setOptions(const QMap<QString,QString>& opts)
{ // This functions remove all options except those with "kde-..."
  // which correspond to externally-sets options (use the value
  // from "opts" if specified
	QMap<QString,QString>	tmpset = m_options;
	m_options = opts;
	for (QMap<QString,QString>::ConstIterator it=tmpset.begin();it!=tmpset.end();++it)
		if (it.key().left(4) == "kde-" && !(m_options.contains(it.key())))
			m_options[it.key()] = it.data();
}

void KPrinter::initOptions(const QMap<QString,QString>& opts)
{ // This function can be used to initialize the KPrinter object just after
  // creation to set some options. Non global options will be propagated to
  // all listed printers (non-global => start with "kde-...")
	m_options = opts;
	if (!m_impl) return;
	for (QMap<QString,QString>::ConstIterator it=opts.begin(); it!=opts.end(); ++it)
		if (it.key().left(4) != "kde-")
			m_impl->broadcastOption(it.key(),it.data());
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

void dumpOptions(const QMap<QString,QString>& opts)
{
	qDebug("********************");
	for (QMap<QString,QString>::ConstIterator it=opts.begin(); it!=opts.end(); ++it)
		qDebug("%s = %s",it.key().latin1(),it.data().latin1());
}

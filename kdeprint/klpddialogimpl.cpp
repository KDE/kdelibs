#include "klpddialogimpl.h"
#include "kprinter.h"
#include "kprintdialog.h"

#include <qfile.h>
#include <qtextstream.h>
#include <qfileinfo.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kconfig.h>

//**********************************************************************
// PrintcapEntry structure
//**********************************************************************

struct PrintcapEntry
{
	QString			name;
	QMap<QString,QString>	options;

	bool parseLine(const QString& s);
	QString option(const QString& key) const { return options[key]; }
};

bool PrintcapEntry::parseLine(const QString& s)
{
	options.clear();
	name = QString::null;
	QStringList	words = QStringList::split(':',s,false);
	if (words.count() > 0)
	{
		name = words[0];
		int	p(-1);
		for (uint i=1;i<words.count();i++)
			if ((p=words[i].find('=')) != -1)
				options.insert(words[i].left(p),words[i].right(words[i].length()-p-1));
		return true;
	}
	else
		return false;
}

//**********************************************************************
// KLpdDialogImpl class
//**********************************************************************

QString getLine(QTextStream& t);

KLpdDialogImpl::KLpdDialogImpl(KPrinter *pr, QObject *parent, const char *name)
: KDialogImpl(pr,parent,name)
{
	dialogflags_ = KPrinter::Preview|KPrinter::OutputToFile|KPrinter::Default;
	standardpages_ = KPrinter::CopiesPage|KPrinter::SizeColorPage;

	entries_.setAutoDelete(true);

	KConfig	conf_("kdeprintrc");
	conf_.setGroup("LPD");
	defaultprinter_ = conf_.readEntry("DefaultPrinter");
}

KLpdDialogImpl::~KLpdDialogImpl()
{
}

KPrinterItemList* KLpdDialogImpl::printersList()
{
	reload();
	return &printers_;
}

void KLpdDialogImpl::printerSelected(KPrintDialog *dlg, int index)
{
	if (index < 0 || index >= (int)(printers_.count())) return;

	KPrinterItem	*item = printers_.at(index);
	PrintcapEntry	*entry = entries_.find(item->printerName());
	dlg->setValidPrinter(true);
	if (entry)
	{
		dlg->setPrinterState(i18n("Printer idle"));
		QString	value = entry->option("rm");
		if (!value.isEmpty())
		{
			dlg->setPrinterType(i18n("Remote printer queue (%1) on %2").arg(entry->option("rp")).arg(value));
		}
		else
			dlg->setPrinterType(i18n("Local printer queue"));
	}
}

KPrinterPropertyDialog* KLpdDialogImpl::buildPropertyDialog(KPrintDialog*, int)
{
	return NULL;	// LPD doesn't support per printer properties !!!
}

void KLpdDialogImpl::reload()
{
	QFileInfo	fi("/etc/printcap");
	if (fi.exists() && fi.lastModified() > checktime_)
	{
		checktime_ = fi.lastModified();
		loadPrintcapFile(fi.absFilePath());
	}

	if (!defaultprinter_.isEmpty())
		setDefault(findPrinter(defaultprinter_));
}

void KLpdDialogImpl::setDefaultPrinter(KPrintDialog *dlg, int index)
{
	KDialogImpl::setDefaultPrinter(dlg,index);
	if (index >= 0 && index < (int)(printers_.count()))
	{
		defaultprinter_ = printers_.at(index)->name();
		KConfig	conf_("kdeprintrc");
		conf_.setGroup("LPD");
		conf_.writeEntry("DefaultPrinter",defaultprinter_);
	}
}

void KLpdDialogImpl::loadPrintcapFile(const QString& filename)
{
	// cleanup and reload everything
	printers_.clear();
	entries_.clear();

	QFile	f(filename);
	if (f.exists() && f.open(IO_ReadOnly))
	{
		QTextStream	t(&f);
		QString	line;
		while (!t.eof())
		{
			line = getLine(t);
			if (!line.isEmpty())
			{
				PrintcapEntry	*entry = new PrintcapEntry();
				if (entry->parseLine(line))
					entries_.insert(entry->name,entry);
				else
					delete entry;
			}
		}
	}
	else
		qWarning("Printcap file (%s) not found or not readable",filename.latin1());

	// convert Printcap entries into KPrinterItem objects
	QDictIterator<PrintcapEntry>	it(entries_);
	QPixmap	pix = UserIcon("kdeprint_pagesize");
	for (;it.current();++it)
	{
		KPrinterItem	*item = new KPrinterItem();
		item->setPrinterName(it.current()->name);
		item->setName(it.current()->name);
		item->setDisplayName(it.current()->name);
		item->setPixmap(pix);
		item->setType(KPrinterItem::Printer|KPrinterItem::Valid|KPrinterItem::Local);
		printers_.inSort(item);
	}
}

//**********************************************************************

QString getLine(QTextStream& t)
{
	QString	line, buf;
	while (!t.eof())
	{
		buf = t.readLine().stripWhiteSpace();
		if (buf.isEmpty() || buf[0] == '#') continue;
		line.append(buf);
		if (line.right(1) == "\\")
		{
			line.truncate(line.length()-1);
			line = line.stripWhiteSpace();
		}
		else
			break;
	}
	return line;
}

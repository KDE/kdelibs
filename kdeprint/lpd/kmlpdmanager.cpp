#include "kmlpdmanager.h"
#include "kmprinter.h"

#include <qfile.h>
#include <qtextstream.h>
#include <qmap.h>

#include <klocale.h>

class PrintcapEntry
{
friend class KMLpdManager;
public:
	bool readLine(const QString& line);
	KMPrinter* createPrinter();
	QString arg(const QString& key) const 	{ return m_args[key]; }
private:
	QString			m_name;
	QMap<QString,QString>	m_args;
};

bool PrintcapEntry::readLine(const QString& line)
{
	QStringList	l = QStringList::split(':',line,false);
	if (l.count() > 0)
	{
		m_name = l[0];
		m_args.clear();
		for (uint i=1; i<l.count(); i++)
		{
			QString	key = l[i].left(2);
			QString	value = l[i].right(l[i].length()-(l[i][2] == '=' ? 3 : 2));
			m_args[key] = value;
		}
		return true;
	}
	return false;
}

KMPrinter* PrintcapEntry::createPrinter()
{
	KMPrinter	*printer = new KMPrinter();
	printer->setName(m_name);
	printer->setPrinterName(m_name);
	printer->setInstanceName(QString::null);
	printer->setState(KMPrinter::Idle);
	printer->setType(KMPrinter::Printer);
	return printer;
}

QString getPrintcapLine(QTextStream& t)
{
	QString	line, buffer;
	while (!t.eof())
	{
		buffer = t.readLine().stripWhiteSpace();
		if (buffer.isEmpty() || buffer[0] == '#')
			continue;
		line.append(buffer);
		if (line.right(1) == "\"")
		{
			line.truncate(line.length()-1);
			line = line.stripWhiteSpace();
		}
		else break;
	}
	return line;
}

//************************************************************************************************

KMLpdManager::KMLpdManager(QObject *parent, const char *name)
: KMManager(parent,name)
{
	m_entries.setAutoDelete(true);
}

KMLpdManager::~KMLpdManager()
{
}

bool KMLpdManager::completePrinter(KMPrinter *printer)
{
	return completePrinterShort(printer);
}

bool KMLpdManager::completePrinterShort(KMPrinter *printer)
{
	PrintcapEntry	*entry = m_entries.find(printer->name());
	if (entry)
	{
		printer->setDescription(i18n("Local printer queue"));
		return true;
	}
	else return false;
}

void KMLpdManager::listPrinters()
{
	m_entries.clear();
	loadPrintcapFile(QString::fromLatin1("/etc/printcap"));

	QDictIterator<PrintcapEntry>	it(m_entries);
	for (;it.current();++it)
	{
		KMPrinter	*printer = it.current()->createPrinter();
		addPrinter(printer);
	}
}

void KMLpdManager::loadPrintcapFile(const QString& filename)
{
	QFile	f(filename);
	if (f.exists() && f.open(IO_ReadOnly))
	{
		QTextStream	t(&f);
		QString		line;
		PrintcapEntry	*entry;
		while (!t.eof())
		{
			line = getPrintcapLine(t);
			if (line.isEmpty())
				continue;
			entry = new PrintcapEntry;
			if (entry->readLine(line))
				m_entries.insert(entry->m_name,entry);
			else
			{
				delete entry;
				break;
			}
		}
	}
}

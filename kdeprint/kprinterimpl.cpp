#include "kprinterimpl.h"
#include "kprinter.h"
#include "kmfactory.h"
#include "kmmanager.h"
#include "kmprinter.h"

void initEditPrinter(KMPrinter *p)
{
	if (!p->isEdited())
	{
		p->setEditedOptions(p->defaultOptions());
		p->setEdited(true);
	}
}

//****************************************************************************************

KPrinterImpl::KPrinterImpl(QObject *parent, const char *name)
: QObject(parent,name)
{
	// initialize file printer
	m_fileprinter = new KMPrinter();
	m_fileprinter->setPrinterName(QString::fromLatin1("File"));
	m_fileprinter->setName("__kdeprint_file");
	m_fileprinter->setDefaultOption("kde-orientation","Portrait");
	m_fileprinter->setDefaultOption("kde-colormode","Color");
	m_fileprinter->setDefaultOption("kde-pagesize",QString::number((int)KPrinter::A4));
}

KPrinterImpl::~KPrinterImpl()
{
	delete m_fileprinter;
}

void KPrinterImpl::preparePrinting(KPrinter*)
{
}

bool KPrinterImpl::printFiles(KPrinter*, const QStringList&)
{
	return false;
}

void KPrinterImpl::broadcastOption(const QString& key, const QString& value)
{
	// force printer listing if not done yet (or reload needed)
	QList<KMPrinter>	*printers = KMFactory::self()->manager()->printerList(false);
	if (printers)
	{
		QListIterator<KMPrinter>	it(*printers);
		for (;it.current();++it)
		{
			initEditPrinter(it.current());
			it.current()->setEditedOption(key,value);
		}
	}

	// update also "file" printer
	initEditPrinter(m_fileprinter);
	m_fileprinter->setEditedOption(key,value);
}

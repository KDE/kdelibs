#include "klpdprinterimpl.h"
#include "kprinter.h"

#include <qfile.h>
#include <kprocess.h>

KLpdPrinterImpl::KLpdPrinterImpl(QObject *parent, const char *name)
: KPrinterImpl(parent,name)
{
}

KLpdPrinterImpl::~KLpdPrinterImpl()
{
}

QString KLpdPrinterImpl::executable()
{
	return QString::fromLatin1("/usr/bin/lpr");
}

bool KLpdPrinterImpl::printFiles(KPrinter *printer, const QStringList& files)
{
	KProcess	proc;
	proc << executable();
	proc << "-P" << printer->printerName() << QString::fromLatin1("-#%1").arg(printer->numCopies());
	bool 	canPrint(false);
	for (QStringList::ConstIterator it=files.begin(); it!=files.end(); ++it)
		if (QFile::exists(*it))
		{
			proc << *it;
			canPrint = true;
		}
		else
			qDebug("File not found: %s",(*it).latin1());
	if (canPrint) return proc.start(KProcess::Block,KProcess::NoCommunication);
	else return false;
}

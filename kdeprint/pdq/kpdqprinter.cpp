#include "kpdqprinter.h"
#include "kpdqdialogimpl.h"
#include "pdqinfos.h"
#include "pdqclass.h"

#include <kprocess.h>

KPdqPrinter::KPdqPrinter()
{
	impl_ = new KPdqDialogImpl(this);
}

KPdqPrinter::~KPdqPrinter()
{
	if (impl_) delete impl_;
}

void KPdqPrinter::translateOptions(const OptionSet& opts)
{
	// orientation
	QString	value = opts["orientation-requested"];
	setOption("qt-orientation",(value == "4" ? "Landscape" : "Portrait"));

	// page order is only for application side

	// nb of copies can be handled by Qt
	wrapper_->setNumCopies(numCopies());

	// ranges is only for application side, but translated
	// into application useable options
	QString	range = opts["qt-range"];
	if (!range.isEmpty())
	{
		QSize	s = rangeToSize(range);
		setOption("qt-from",QString::number(s.width()));
		setOption("qt-to",QString::number(s.height()));
	}
	else
	{
		qtoptions_.remove("qt-from");
		qtoptions_.remove("qt-to");
		qtoptions_.remove("qt-range");
	}

	// call base class function (necessary)
	KPrinter::translateOptions(opts);
}

bool KPdqPrinter::printFiles(const QStringList& files)
{
	KPrinter::printFiles(files);

	KProcess proc;
	PDQPrinter	*pr = ((KPdqDialogImpl*)impl_)->printerWithOptions(printerName(),qtoptions_);

	proc.setExecutable("pdq");
	if (!printerName().isEmpty()) proc << "-P" << printerName();
	for (QStringList::ConstIterator it=pr->driver_options_.begin(); it!=pr->driver_options_.end();++it)
		proc << "-o" << *it;
	for (QMap<QString,QString>::ConstIterator it=pr->driver_args_.begin(); it!=pr->driver_args_.end(); ++it)
		proc << "-a" << QString::fromLatin1("%1=%2").arg(it.key()).arg(it.data());
	for (QStringList::ConstIterator it=pr->interface_options_.begin(); it!=pr->interface_options_.end();++it)
		proc << "-O" << *it;
	for (QMap<QString,QString>::ConstIterator it=pr->interface_args_.begin(); it!=pr->interface_args_.end(); ++it)
		proc << "-A" << QString::fromLatin1("%1=%2").arg(it.key()).arg(it.data());
	for (QStringList::ConstIterator it = files.begin(); it != files.end(); ++it)
		proc << (*it);

	if (files.count() > 0)
		return (proc.start(KProcess::Block,KProcess::NoCommunication));
	return false;
}

int KPdqPrinter::copyFlags() const
{
	if (pageselection_ == KPrinter::ApplicationSide)
		return (KPrinter::copyFlags() & ~(KDialogImpl::Collate));
	else
		return 0;
}

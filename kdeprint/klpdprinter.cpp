#include "klpdprinter.h"
#include "klpddialogimpl.h"

#include <kprocess.h>

KLpdPrinter::KLpdPrinter()
{
	impl_ = new KLpdDialogImpl(this);
}

KLpdPrinter::~KLpdPrinter()
{
	if (impl_) delete impl_;
}

void KLpdPrinter::translateOptions(const OptionSet& opts)
{
	// orientation
	QString	value = opts["orientation-requested"];
	setOption("qt-orientation",(value == "4" ? "Landscape" : "Portrait"));

	// page order is only for application side

	// nb of copies can be handled by Qt
	wrapper_->setNumCopies(numCopies());

	// ranges is only for application side, but translated it
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

bool KLpdPrinter::printFiles(const QStringList& files)
{
	KPrinter::printFiles(files);

	KProcess proc;

	proc.setExecutable("lpr");
	if (!printerName().isEmpty()) proc << "-P" << printerName();
	for (QStringList::ConstIterator it = files.begin(); it != files.end(); ++it)
		proc << (*it);

	if (files.count() > 0)
		return (proc.start(KProcess::Block,KProcess::NoCommunication));
	return false;
}

int KLpdPrinter::copyFlags() const
{
	if (pageselection_ == KPrinter::ApplicationSide)
		return (KPrinter::copyFlags() & ~(KDialogImpl::Collate));
	else
		return 0;
}

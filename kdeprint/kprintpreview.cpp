#include "kprintpreview.h"

#include <klibloader.h>
#include <ktrader.h>
#include <ktrader.h>
#include <kapp.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kmessagebox.h>

KPrintPreview::KPrintPreview(QWidget *parent)
: KParts::MainWindow(parent,"KPrintPreview",WType_Modal|WType_TopLevel|WStyle_Dialog)
{
	setXMLFile(locate("config","ui/kprintpreviewui.rc"));
	setHelpMenuEnabled(false);

	new KAction(i18n("Print"),"fileprint",Qt::Key_Return,this,SLOT(accept()),actionCollection(),"continue_print");
	new KAction(i18n("Cancel"),"stop",Qt::Key_Escape,this,SLOT(reject()),actionCollection(),"stop_print");

	gvpart_ = 0;
	status_ = false;

	// ask the trader for service handling postscript
	KTrader::OfferList	offers = KTrader::self()->query("application/postscript","'KParts/ReadOnlyPart' in ServiceTypes");
	for (KTrader::OfferList::ConstIterator it = offers.begin(); it != offers.end(); ++it)
	{
		KService::Ptr	service = *it;
		KLibFactory	*factory = KLibLoader::self()->factory(service->library().latin1());
		if (factory)
		{
			gvpart_ = (KParts::ReadOnlyPart*)factory->create(this,"gvpart","KParts::ReadOnlyPart");
			break;
		}
	}
	if (!gvpart_)
	{
		// nothing has been found, try to load directly KGhostview part
		KLibFactory	*factory = KLibLoader::self()->factory("libkghostview");
		if (factory)
			gvpart_ = (KParts::ReadOnlyPart*)factory->create(this,"gvpart","KParts::ReadOnlyPart");
	}

	if (gvpart_)
	{
		setCentralWidget(gvpart_->widget());
		createGUI(gvpart_);
	}
}

KPrintPreview::~KPrintPreview()
{
	if (gvpart_) delete gvpart_;
}

void KPrintPreview::openFile(const QString& file)
{
	gvpart_->openURL(KURL(file));
	QSize	s(gvpart_->widget()->sizeHint());
	//resize(QSize(s.width()+40,s.height()+70));
	resize(QMAX(s.width(),760),QMAX(s.height(),750));

	setCaption(i18n("Print preview"));
}

void KPrintPreview::accept()
{
	done(true);
}

void KPrintPreview::reject()
{
	done(false);
}

void KPrintPreview::done(bool st)
{
	status_ = st;
	hide();
	kapp->exit_loop();
}

bool KPrintPreview::isValid() const
{
	return (gvpart_ != 0);
}

void KPrintPreview::exec(const QString& file)
{
	if (isValid())
	{
		show();
		if (!file.isNull()) openFile(file);
		kapp->enter_loop();
	}
	else
		status_ = (KMessageBox::warningYesNo(this,i18n("KDE was unable to locate an appropriate object for print previewing.\nDo you want to continue printing anyway ?")) == KMessageBox::Yes);
}

void KPrintPreview::closeEvent(QCloseEvent *e)
{
	e->accept();
	reject();
}

bool KPrintPreview::preview(const QString& file)
{
	KPrintPreview	dlg(0);
	dlg.exec(file);
	return dlg.status();
}
#include "kprintpreview.moc"

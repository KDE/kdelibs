#include "kpdqpage.h"
#include "kpdqview.h"
#include "kpdqoptionview.h"
#include "pdqclass.h"

#include <qlayout.h>
#include <klocale.h>

KPdqPage::KPdqPage(PDQPrinter *pr, PDQDriver *dr, PDQInterface *in, QWidget *parent, const char *name)
: KPrintDialogPage(parent,name)
{
	printer_ = pr;
	driver_ = dr;
	interface_ = in;

	view_ = new KPdqView(this);
	optview_ = new KPdqOptionView(this);
	view_->init(printer_,driver_,interface_);

	QVBoxLayout	*main_ = new QVBoxLayout(this, 10, 10);
	main_->addWidget(view_,3);
	main_->addWidget(optview_,1);

	connect(view_,SIGNAL(selectionChanged(QListViewItem*)),optview_,SLOT(optionSelected(QListViewItem*)));
}

KPdqPage::~KPdqPage()
{
}

void KPdqPage::setOptions(const OptionSet& opts)
{
	view_->setOptions(opts);
}

void KPdqPage::getOptions(OptionSet& opts, bool incldef)
{
	view_->getOptions(opts,incldef);
}

QString KPdqPage::pageTitle()
{
	return i18n("PDQ options");
}
